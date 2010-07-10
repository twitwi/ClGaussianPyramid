#include <math.h>
#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

#include <cv.h>
#include <highgui.h>

#include <clgp.h>

#define SCALE_OFFSET_X(scale, width, height) \
    ((scale != 0)*width)
#define SCALE_OFFSET_Y(scale, width, height) \
    ((scale >= 2) ? (int)((( (1.f - powf(0.5f, (float)scale)) / (1.f-0.5f) ) - 1.f)*(float)height) : 0) /* Make libcuda.so segfault????? */

int scale_offset_y(int scale, int width, int height) /* Tmp replacement */
{
    float res = 0.f;

    if (scale < 2) {
        return 0;
    }

    for (int s = 2; s <= scale; s++) {
        res += height/(1<<(s-1));
    }

    return (int)res;
}

int
main(int argc, char *argv[])
{
    cl_int err = 0;

    cl_platform_id platforms[1];
    cl_uint n_platforms = 0;
    int myplatform = 0;
    cl_platform_id platform;
    cl_device_id devices[1];
    cl_uint n_devs = 0;
    int mydevice = 0;
    cl_device_id device = 0;
    cl_context context = 0;
    cl_command_queue queue = 0;

    IplImage *input = NULL, *tmp = NULL, *output = NULL;

    cl_image_format imageformat = {CL_BGRA, CL_UNSIGNED_INT8};
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1}; /* To update when we got image size */
    cl_mem climage_input, climage_pyramid[16];
    
    int scale = 0, maxscale = 0;

    /* OpenCL init, many should go in a clgp_init() function */
    /* Enumerate platforms */
    err =
        clGetPlatformIDs(
                1,
                platforms,
                &n_platforms);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not check for platforms\n");
        exit(1);
    }

    /* Choose a platform */
    platform = platforms[myplatform];

    /* Enumerate devices */
    err =
        clGetDeviceIDs(
                platform,
                CL_DEVICE_TYPE_GPU,
                1,
                devices,
                &n_devs);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not check for devices\n");
        exit(1);
    }
    if (n_devs == 0) {
        fprintf(stderr, "No device available on this host\n");
        exit(1);
    }

    /* Choose a device */
    device = devices[mydevice];

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
    if (clgp_init(context, queue) != 0) {
        fprintf(stderr, "Could not init clgp library\n");
        exit(1);
    }

    /* Load image */
    input = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
    if (input == NULL) {
        fprintf(stderr, "Could not load file %s\n", argv[1]);
        exit(1);
    }

    /* Convert to an acceptable OpenCL format (we don't do {RGB, UINT8} */
    tmp = 
        cvCreateImage(
                cvSize(input->width, input->height), 
                input->depth, 
                4);
    cvCvtColor(input, tmp, CV_BGR2BGRA);
    cvReleaseImage(&input);
    input = tmp;
    tmp = NULL;

    climage_input =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY,
                &imageformat,
                input->width,
                input->height,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_input\n");
        exit(1);
    }

    maxscale = clgp_maxscale(input->width, input->height);
    for (scale = 0; scale < maxscale; scale++) {
        climage_pyramid[scale] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &imageformat,
                    input->width/(1<<scale),
                    input->height/(1<<scale),
                    0,
                    NULL,
                    &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Could not allocate climage_pyramid[%i]\n", scale);
            exit(1);
        }
    }

    region[0] = input->width;
    region[1] = input->height;
    err =
        clEnqueueWriteImage(
                queue,
                climage_input,
                CL_TRUE,
                origin,
                region,
                input->widthStep,
                0,
                input->imageData,
                0,
                NULL,
                NULL);
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy input data on device (%i)\n", err);
        exit(1);
    }


    /* At last, call our pyramid function */
    clgp_pyramid(
            climage_pyramid, 
            climage_input, 
            input->width, 
            input->height, 
            maxscale);


    /* Create output image */
    output = 
        cvCreateImage(
                cvSize(input->width*1.5, input->height),
                input->depth, 
                4);

    /* Retrieve images */
    for (scale = 0; scale < maxscale; scale++) {
        region[0] = input->width/(1<<scale);
        region[1] = input->height/(1<<scale);
        err = 
            clEnqueueReadImage(
                    queue,
                    climage_pyramid[scale],
                    CL_TRUE,
                    origin,
                    region,
                    output->widthStep,
                    0,
                    ((char *)output->imageData + SCALE_OFFSET_X(scale, input->width, input->height)*4 + scale_offset_y(scale, input->width, input->height)*output->widthStep),
                    0,
                    NULL,
                    NULL);
        clFinish(queue);
        if (err != CL_SUCCESS) {
            fprintf(stderr, 
                    "Could not copy scale %i data on host (%i)\n", scale, err);
            exit(1);
        }
    }


    /* Release the clgp library */
    clgp_release();


    /* Release device ressources */
    clReleaseMemObject(climage_input);
    for (scale = 0; scale < maxscale; scale++) {
        clReleaseMemObject(climage_pyramid[scale]);
    }

    clReleaseContext(context);
    clReleaseCommandQueue(queue);


    /* Show results */
    cvNamedWindow("gaussian pyramid", CV_WINDOW_AUTOSIZE);
    cvShowImage("gaussian pyramid", output);
    cvWaitKey(0);
    cvDestroyWindow("gaussian pyramid");

    cvReleaseImage(&input);
    cvReleaseImage(&output);

    return 0;
}

