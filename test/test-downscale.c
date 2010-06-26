#include <stdio.h>
#include <stdlib.h>

#include <CL/cl.h>

#include <cv.h>
#include <highgui.h>

#include <clgp.h>
#include <downscale.h>


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

    IplImage *scale1 = NULL, *scale2 = NULL, *scale4 = NULL, *tmp = NULL;

    cl_image_format imageformat = {CL_BGRA, CL_UNSIGNED_INT8};
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1}; /* To update when we got image size */
    cl_mem climage_scale1 = 0, climage_scale2 = 0, climage_scale4 = 0;

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
    scale1 = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
    if (scale1 == NULL) {
        fprintf(stderr, "Could not load file %s\n", argv[1]);
        exit(1);
    }

    /* Convert to an acceptable OpenCL format (we don't do {RGB, UINT8} */
    tmp = 
        cvCreateImage(
                cvSize(scale1->width, scale1->height), 
                scale1->depth, 
                4);
    cvCvtColor(scale1, tmp, CV_BGR2BGRA);
    cvReleaseImage(&scale1);
    scale1 = tmp;
    tmp = NULL;

    climage_scale1 =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY,
                &imageformat,
                scale1->width,
                scale1->height,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_scale1\n");
        exit(1);
    }

    climage_scale2 =
        clCreateImage2D(
                context,
                CL_MEM_WRITE_ONLY,
                &imageformat,
                scale1->width/2,
                scale1->height/2,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_scale2\n");
        exit(1);
    }

    climage_scale4 =
        clCreateImage2D(
                context,
                CL_MEM_WRITE_ONLY,
                &imageformat,
                scale1->width/4,
                scale1->height/4,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_scale4\n");
        exit(1);
    }

    region[0] = scale1->width;
    region[1] = scale1->height;
    err =
        clEnqueueWriteImage(
                queue,
                climage_scale1,
                CL_TRUE,
                origin,
                region,
                scale1->widthStep,
                0,
                scale1->imageData,
                0,
                NULL,
                NULL);
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy scale1 data on device (%i)\n", err);
        exit(1);
    }


    /* At last, call our downscale function */
    clgp_downscale(
            climage_scale2, 
            climage_scale1,
            scale1->width,
            scale1->height);
    clgp_downscale(
            climage_scale4, 
            climage_scale2,
            scale1->width/2,
            scale1->height/2);


    /* Retrieve scale2 image */
    scale2 = 
        cvCreateImage(
                cvSize(scale1->width/2, scale1->height/2), 
                scale1->depth, 
                4);
    region[0] = scale2->width;
    region[1] = scale2->height;
    err = 
        clEnqueueReadImage(
                queue,
                climage_scale2,
                CL_TRUE,
                origin,
                region,
                scale2->widthStep,
                0,
                scale2->imageData,
                0,
                NULL,
                NULL);
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy scale2 data on host (%i)\n", err);
        exit(1);
    }

    /* Retrieve scale4 image */
    scale4 = 
        cvCreateImage(
                cvSize(scale1->width/4, scale1->height/4), 
                scale1->depth, 
                4);
    region[0] = scale4->width;
    region[1] = scale4->height;
    err = 
        clEnqueueReadImage(
                queue,
                climage_scale4,
                CL_TRUE,
                origin,
                region,
                scale4->widthStep,
                0,
                scale4->imageData,
                0,
                NULL,
                NULL);
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy scale4 data on host (%i)\n", err);
        exit(1);
    }


    /* Release the clgp library */
    clgp_release();


    /* Release device ressources */
    clReleaseMemObject(climage_scale1);
    clReleaseMemObject(climage_scale2);
    clReleaseMemObject(climage_scale4);

    clReleaseContext(context);
    clReleaseCommandQueue(queue);


    /* Show results */
    cvNamedWindow("scale1 image", CV_WINDOW_AUTOSIZE);
    cvShowImage("scale1 image", scale1);
    cvNamedWindow("scale2 image", CV_WINDOW_AUTOSIZE);
    cvShowImage("scale2 image", scale2);
    cvNamedWindow("scale4 image", CV_WINDOW_AUTOSIZE);
    cvShowImage("scale4 image", scale4);
    cvWaitKey(0);
    cvDestroyWindow("scale1 image");
    cvDestroyWindow("scale2 image");
    cvDestroyWindow("scale4 image");

    cvReleaseImage(&scale1);
    cvReleaseImage(&scale2);
    cvReleaseImage(&scale4);

    return 0;
}

