#include <sys/time.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#ifdef HAVE_GETOPT_LONG
#include <getopt.h>
#endif

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include <clgp.h>
#include <utils.h>

#include <wand/MagickWand.h>

#define BUILD_ITERATION_NB 10

#ifdef HAVE_GETOPT_LONG
static struct option longopts[] = {
    { "colormap", required_argument, 0, 'c' },
    { "devicetype", required_argument, 0, 'd' },
    { "help", no_argument, 0, 'h' },
    { "pyramidtype", required_argument, 0, 'p' },
    { 0, 0, 0, 0 }
};
#endif

/* Abstraction to allow the use of any pyramid type in this program: you don't
 * have to do this normaly! */
struct pyramid_implementation {
    size_t (*layoutWidth)(size_t width);
    size_t (*levelWidth)(size_t width, size_t level);
    size_t (*levelHeight)(size_t height, size_t level);
    size_t (*layoutOriginX)(size_t level, size_t width, size_t height);
    size_t (*layoutOriginY)(size_t level, size_t width, size_t height);
    size_t (*maxlevel)(size_t width, size_t height);
    int (*enqueuePyramid)(cl_command_queue command_queue, cl_kernel *kernels, cl_mem pyramid_image[], cl_mem input_image, size_t maxlevel);
};

static size_t
pyramidLayoutWidth(size_t width)
{
    return 2*width;
}
static size_t 
pyramidLevelWidth(size_t width, size_t level) 
{
    return width >> level;
} 
static size_t 
pyramidLevelHeight(size_t height, size_t level) 
{
    return height >> level;
} 
static size_t
pyramidLayoutOriginX(size_t level, size_t width, size_t height)
{
    return ((level == 0) \
        ? 0 \
        : (size_t)(((1.f - powf(0.5f, (float)(level))) / (1.f-0.5f)) *(float)width));
}
static size_t
pyramidLayoutOriginY(size_t level, size_t width, size_t height)
{
    return 0;
}
static size_t
pyramidMaxlevel(size_t width, size_t height)
{
    return clgpMaxlevel(width, height) - 4;
}
const struct pyramid_implementation pyramid = {
    pyramidLayoutWidth,
    pyramidLevelWidth,
    pyramidLevelHeight,
    pyramidLayoutOriginX,
    pyramidLayoutOriginY,
    pyramidMaxlevel,
    clgpEnqueuePyramid,
};

static size_t
pyramidHalfOctaveLayoutWidth(size_t width)
{
    return 3*width;
}
static size_t 
pyramidHalfOctaveLevelWidth(size_t width, size_t level) 
{
    return width >> (level>>1);
} 
static size_t 
pyramidHalfOctaveLevelHeight(size_t height, size_t level) 
{
    return height >> (level>>1);
} 
static size_t
pyramidHalfOctaveLayoutOriginX(size_t level, size_t width, size_t height)
{
    return ((level == 0) \
        ? 0 \
        : (size_t)((( (1.f - powf(0.5f, (float)(level>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width));
}
static size_t
pyramidHalfOctaveLayoutOriginY(size_t level, size_t width, size_t height)
{
    return ((level <= 2) ? 0 : (level & 0x1)*(height>>(level>>1))); 
}
static size_t
pyramidHalfOctaveMaxlevel(size_t width, size_t height)
{
    return clgpMaxlevelHalfOctave(width, height) - 4;
}
const struct pyramid_implementation pyramidHalfOctave = {
    pyramidHalfOctaveLayoutWidth,
    pyramidHalfOctaveLevelWidth,
    pyramidHalfOctaveLevelHeight,
    pyramidHalfOctaveLayoutOriginX,
    pyramidHalfOctaveLayoutOriginY,
    pyramidHalfOctaveMaxlevel,
    clgpEnqueuePyramidHalfOctave,
};

static size_t
pyramidSqrt2LayoutWidth(size_t width)
{
    return 2*width;
}
static size_t
pyramidSqrt2LevelWidth(size_t width, size_t level)
{
    return width >> ((level+1)>>1);
}
static size_t
pyramidSqrt2LevelHeight(size_t height, size_t level)
{
    return height >> (level>>1);
}
static size_t
pyramidSqrt2LayoutOriginX(size_t level, size_t width, size_t height)
{
    return ((level == 0) \
        ? 0 \
        : ((size_t)(( (1.f - powf(0.5f, (float)(level>>1)) ) / (1.f-0.5f) )*(float)(width/2)) + width));
}
static size_t
pyramidSqrt2LayoutOriginY(size_t level, size_t width, size_t height)
{
    return ((level <= 2) \
        ? 0 \
        : (int)(( (1.f - powf(0.5f, (float)((level-1)>>1)) ) / (1.f-0.5f) )*(float)(height/2)));
}
static size_t
pyramidSqrt2Maxlevel(size_t width, size_t height)
{
    return clgpMaxlevelHalfOctave(width, height) - 7;
}
const struct pyramid_implementation pyramidSqrt2 = {
    pyramidSqrt2LayoutWidth,
    pyramidSqrt2LevelWidth,
    pyramidSqrt2LevelHeight,
    pyramidSqrt2LayoutOriginX,
    pyramidSqrt2LayoutOriginY,
    pyramidSqrt2Maxlevel,
    clgpEnqueuePyramidSqrt2,
};


void
usage(void)
{
    fprintf(stderr, 
            "Usage: test-pyramid [OPTIONS] IMAGE_PATH\n"
                " -c, --colormap=CHANNEL_ORDER     rgba, intensity\n"
                " -d, --devicetype=DEVICE_TYPE     cpu, gpu\n"
                " -p, --pyramidtype=PYRAMID_TYPE   pyramid, halfoctave, sqrt2\n"
                " -h                               display help\n");
}

int
main(int argc, char *argv[])
{
    cl_int err = 0;

    int devicetype = 0;
    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_kernel *clgpkernels = NULL;

    const struct pyramid_implementation *impl = &pyramid;

    MagickWand *input_wand = NULL, *pyramid_wand = NULL;
    const char *magickpixelmap = "RGBA";
    char *input_data = NULL, *pyramid_data;
    unsigned int input_width = 0, pyramid_width = 0;
    unsigned int input_height = 0, pyramid_height = 0;
    unsigned int input_nbchannels = 0, pyramid_nbchannels = 0;

    cl_image_format imageformat = {CL_RGBA, CL_UNORM_INT8};
    cl_mem input_climage, pyramid_climage[32];
    size_t maxlevel = 0, level = 0;
    
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    struct timeval start, stop;
    double compute_time = 0., total_time = 0.;
    
    int i = 0;

    
    /* Parse command line arguments */
    opterr = 0;
#ifdef HAVE_GETOPT_LONG
    while ((i = getopt_long(argc, argv, ":c:d:hp:", longopts, NULL)) != -1) {
#else
    while ((i = getopt(argc, argv, ":c:d:hp:")) != -1) {
#endif
        switch(i) {
            case 'c':
                if (strncmp(optarg, "rgba", 5) == 0) {
                    magickpixelmap = "RGBA";
                    imageformat.image_channel_order = CL_RGBA;
                }
                else if (strncmp(optarg, "intensity", 10) == 0) {
                    magickpixelmap = "I";
                    imageformat.image_channel_order = CL_INTENSITY;
                }
                else {
                    fprintf(stderr, "-c: invalid channel order\n");
                    opterr++;
                }
                break;
            case 'd':
                if (strncmp(optarg, "gpu", 4) == 0) {
                    devicetype = 0;
                }
                else if (strncmp(optarg, "cpu", 4) == 0) {
                    devicetype = 1;
                }
                else {
                    fprintf(stderr, "-d: invalid device type\n");
                    opterr++;
                }
                break;
            case 'h':
                usage();
                exit(0);
                break;
            case 'p':
                if (strncmp(optarg, "pyramid", 8) == 0) {
                    impl = &pyramid;
                }
                else if (strncmp(optarg, "halfoctave", 11) == 0) {
                    impl = &pyramidHalfOctave;
                }
                else if (strncmp(optarg, "sqrt2", 6) == 0) {
                    impl = &pyramidSqrt2;
                }
                else {
                    fprintf(stderr, "-p: invalid pyramid type\n");
                    opterr++;
                }
                break;
            case ':':
                fprintf(stderr, "Option -%c requires an operand\n", optopt);
                opterr++;
                break;
            case '?':
                fprintf(stderr, "Unrecognized option: -%c\n", optopt);
                opterr++;
                break;
        }
        if (opterr != 0) {
            usage();
            exit(1);
        }
    }


    /* ImageMagick init */
    MagickWandGenesis();

    /* OpenCL init, using our utils functions */
    if (devicetype == 0) {
        clgpFirstGPU(&device);
    }
    else if (devicetype == 1) {
        clgpFirstCPU(&device);
    }
    if (device == NULL) {
        fprintf(stderr, "No device available\n");
        exit(1);
    }

    /* Create a context on this device */
    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not create a context for the device\n");
        exit(1);
    }

    /* Create a command queue for the library */
    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not create a command queue for the device\n");
        exit(1);
    }

    /* Initialize the clgp library */
    if (clgpInit(context, &clgpkernels) != 0) {
        fprintf(stderr, "Could not init clgp library\n");
        exit(1);
    }

    /* Load image */
    input_wand = NewMagickWand();
    if (MagickReadImage(input_wand, argv[optind]) != MagickTrue) {
        fprintf(stderr, "Could not load input image\n");
        exit(1);
    }
    input_width = MagickGetImageWidth(input_wand);
    input_height = MagickGetImageHeight(input_wand);
    input_nbchannels = strlen(magickpixelmap);
    input_data = 
        malloc(input_width*input_height*input_nbchannels*sizeof(char));
    MagickExportImagePixels(
            input_wand,
            0,
            0,
            input_width,
            input_height,
            magickpixelmap,
            CharPixel,
            input_data);

    /* Create pyramid image */
    pyramid_wand = NewMagickWand();
    pyramid_width = impl->layoutWidth(input_width);
    pyramid_height = input_height;
    pyramid_nbchannels = input_nbchannels;
    pyramid_data = 
        malloc(pyramid_width*pyramid_height*input_nbchannels*sizeof(char));
    memset(pyramid_data, 0, pyramid_width*pyramid_height*input_nbchannels*sizeof(char));

    /* Create buffers on device */
    input_climage =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY,
                &imageformat,
                input_width,
                input_height,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate input_climage\n");
        exit(1);
    }

    maxlevel = impl->maxlevel(input_width, input_height);
    for (level = 0; level < maxlevel; level++) {
        pyramid_climage[level] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &imageformat,
                    impl->levelWidth(input_width, level),
                    impl->levelHeight(input_height, level),
                    0,
                    NULL,
                    &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Could not allocate pyramid_climage[%lu]\n", level);
            exit(1);
        }
    }


    /* Send input image on device */
    gettimeofday(&start, NULL);
    region[0] = input_width;
    region[1] = input_height;
    err = 
        clEnqueueWriteImage(
                queue,
                input_climage,
                CL_TRUE,
                origin,
                region,
                input_width*input_nbchannels*sizeof(char),
                0,
                input_data,
                0,
                NULL,
                NULL);
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy input data on device (%d)\n", err);
        exit(1);
    }
    gettimeofday(&stop, NULL);
    total_time = 
        (stop.tv_sec-start.tv_sec)*1000. + (stop.tv_usec-start.tv_usec)/1000.;


    /* At last, call our pyramid function */
    gettimeofday(&start, NULL);
    for (i = 0; i < BUILD_ITERATION_NB; i++) {
        impl->enqueuePyramid(
                queue,
                clgpkernels,
                pyramid_climage, 
                input_climage,
                maxlevel);
    }
    clFinish(queue);
    gettimeofday(&stop, NULL);
    compute_time = 
        (stop.tv_sec-start.tv_sec)*1000. + (stop.tv_usec-start.tv_usec)/1000.;
    compute_time /= BUILD_ITERATION_NB;
    total_time += compute_time;


    /* Retrieve images */
    gettimeofday(&start, NULL);
    for (level = 0; level < maxlevel; level++) {
        region[0] = impl->levelWidth(input_width, level);
        region[1] = impl->levelHeight(input_height, level);
        err = 
            clEnqueueReadImage(
                    queue,
                    pyramid_climage[level],
                    CL_TRUE,
                    origin,
                    region,
                    pyramid_width*pyramid_nbchannels*sizeof(char),
                    0,
                    (void *)((char *)((char *)pyramid_data + impl->layoutOriginY(level, input_width, input_height)*pyramid_width*pyramid_nbchannels) + impl->layoutOriginX(level, input_width, input_height)*pyramid_nbchannels),
                    0,
                    NULL,
                    NULL);
    }
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, 
                "Could not copy pyramid data on host (%d)\n", err);
        exit(1);
    }
    gettimeofday(&stop, NULL);
    total_time += 
        (stop.tv_sec-start.tv_sec)*1000. + (stop.tv_usec-start.tv_usec)/1000.;


    /* Release the clgp library */
    clgpRelease(clgpkernels);


    /* Release device ressources */
    clReleaseMemObject(input_climage);
    for (level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climage[level]);
    }

    clReleaseContext(context);
    clReleaseCommandQueue(queue);


    /* Show results */
    printf(" * Pyramid time: %f ms\n", compute_time);
    printf(" * Total time: %f ms\n", total_time);
    /* Display */
    MagickConstituteImage(
            pyramid_wand, 
            pyramid_width, 
            pyramid_height, 
            magickpixelmap, 
            CharPixel, 
            pyramid_data);
    MagickSetImageOpacity(pyramid_wand, 1.0);
    MagickDisplayImages(pyramid_wand, NULL);

    /* Free images and ImageMagick */
    free(input_data);
    free(pyramid_data);
    DestroyMagickWand(input_wand);
    DestroyMagickWand(pyramid_wand);

    MagickWandTerminus();


    return 0;
}

