#include <assert.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include <clgp/clgp.h>
#include <clgp/utils.h>

#include <cv.h>

#include "layoututils.h"

const cl_image_format clgpimageformat = {CL_RGBA, CL_UNSIGNED_INT8};

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct gausspyramid_module {
    Framework framework;

    cl_command_queue command_queue;
    cl_kernel *clgpkernels;
};

struct gausspyramid_module *
GaussPyramid__v__create(Framework f)
{
    struct gausspyramid_module *res = NULL;
    
    res = malloc(sizeof(struct gausspyramid_module));
    assert(res != NULL);
    memset(res, 0, sizeof(struct gausspyramid_module));
    res->framework = f;

    return res;
}

void
GaussPyramid__v__init(struct gausspyramid_module *module)
{
    cl_int clerr = 0;
    cl_device_id device = NULL;
    cl_context context = NULL;
    int err = 0;

    clgpMaxflopsGPU(&device);
    assert(device != NULL);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &clerr);
    assert(err == CL_SUCCESS);

    module->command_queue = clCreateCommandQueue(context, device, 0, &clerr);
    assert(err == CL_SUCCESS);

    err = clgpInit(context, &module->clgpkernels);
    assert(err == 0);

    clReleaseContext(context);
}

void
GaussPyramid__v__stop(struct gausspyramid_module *module)
{
    clgpRelease(module->clgpkernels);
    clReleaseCommandQueue(module->command_queue);
    free(module);
}

static void
outputboth(
        struct gausspyramid_module *module, 
        const unsigned char *rgbadata, 
        int width,
        int height)
{
    void *output_rgba[] = {
        "outputRGBA",
        "char *", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    void *output_rgb[] = {
        "outputRGB",
        "char *", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    int pyramid_width = 3*width, pyramid_height = height;
    unsigned char *rgbdata = NULL;

    int x = 0, y  = 0;

    output_rgba[2] = &rgbadata;
    output_rgba[4] = &pyramid_width;
    output_rgba[6] = &pyramid_height;
    module->framework("emit", output_rgba);

    rgbdata = malloc(pyramid_width*height*3*sizeof(unsigned char));
    assert(rgbdata != NULL);
    for(y = 0; y < height; y++) {
        for(x = 0; x < 3*width; x++) {
            rgbdata[y*3*3*width + x*3 + 0] = rgbadata[y*3*width*4 + x*4 + 0];
            rgbdata[y*3*3*width + x*3 + 1] = rgbadata[y*3*width*4 + x*4 + 1];
            rgbdata[y*3*3*width + x*3 + 2] = rgbadata[y*3*width*4 + x*4 + 2];
        }
    }

    output_rgb[2] = &rgbdata;
    output_rgb[4] = &pyramid_width;
    output_rgb[6] = &pyramid_height;
    module->framework("emit", output_rgb);

    free(rgbdata);
}

void
GaussPyramid__v__event__v__inputRgba(
        struct gausspyramid_module *module,
        unsigned char *data,
        int width,
        int height)
{
    cl_int clerr = 0;
    int clgperr = 0;

    cl_context context = NULL;
    cl_mem input_climage, pyramid_climage[32];
    int maxlevel = 0, level = 0;

    unsigned char *outdata = NULL;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    clerr =
        clGetKernelInfo(
                module->clgpkernels[0],
                CL_KERNEL_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    input_climage =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY,
                &clgpimageformat,
                width,
                height,
                0,
                NULL,
                &clerr);
    assert(clerr == CL_SUCCESS);

    maxlevel = clgpMaxlevelHalfOctave(width, height);
    for (level = 0; level < maxlevel; level++) {
        pyramid_climage[level] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &clgpimageformat,
                    width >> (level>>1),
                    height >> (level>>1),
                    0,
                    NULL,
                    &clerr);
        assert(clerr == CL_SUCCESS);
    }

    region[0] = width;
    region[1] = height;
    clerr =
        clEnqueueWriteImage(
                module->command_queue,
                input_climage,
                CL_TRUE,
                origin,
                region,
                0,
                0,
                data,
                0,
                NULL,
                NULL);
    assert(clerr == CL_SUCCESS);

    clgperr =
        clgpEnqueuePyramidHalfOctave(
            module->command_queue,
            module->clgpkernels,
            pyramid_climage,
            input_climage,
            maxlevel);
    assert(clgperr == 0);

    outdata = malloc(3*width*height*4*sizeof(unsigned char));
    for (level = 0; level < maxlevel; level++) {
        region[0] = width >> (level>>1);
        region[1] = height >> (level>>1);
        clerr =
            clEnqueueReadImage(
                    module->command_queue,
                    pyramid_climage[level],
                    CL_TRUE,
                    origin,
                    region,
                    3*width*4*sizeof(unsigned char),
                    0,
                    (void *)((unsigned char *)((char *)outdata + LEVEL_ORIGIN_Y(level, width, height)*3*width*4) + LEVEL_ORIGIN_X(level, width, height)*4),
                    0,
                    NULL,
                    NULL);
        assert(clerr == CL_SUCCESS);
    }

    clReleaseMemObject(input_climage);
    for (level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climage[level]);
    }

    outputboth(module, outdata, width, height);
    free(outdata);
}

void
GaussPyramid__v__event__v__inputRgb(
        struct gausspyramid_module *module,
        unsigned char *data,
        int width,
        int height)
{
    unsigned char *data_rgba = NULL;

    int x = 0, y = 0;

    data_rgba = malloc(width*height*4*sizeof(unsigned char));
    assert(data_rgba != NULL);

    for (y = 0; y < height; y++) {
        for (x = 0; x < width; x++) {
            data_rgba[y*width*4 + x*4 + 0] = data[y*width + x*3 + 0];
            data_rgba[y*width*4 + x*4 + 1] = data[y*width + x*3 + 1];
            data_rgba[y*width*4 + x*4 + 2] = data[y*width + x*3 + 2];
            data_rgba[y*width*4 + x*4 + 3] = 255;
        }
    }

    GaussPyramid__v__event__v__inputRgba(module, data_rgba, width, height);
    free(data_rgba);
}

void
GaussPyramid__v__event__v__input(
        struct gausspyramid_module *module,
        IplImage *image)
{
    IplImage *rgba = NULL;

    rgba = cvCreateImage(cvSize(image->width, image->height), IPL_DEPTH_8U, 4);
    assert(rgba != NULL);
    cvCvtColor(image, rgba, CV_RGB2RGBA);
    GaussPyramid__v__event__v__inputRgba(
            module, 
            (unsigned char *)rgba->imageData, 
            image->width, 
            image->height);
    cvReleaseImage(&rgba);
}
