#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <CL/cl.h>

#include <cv.h>
#include <highgui.h>

#include <clgp.h>
#include <utils.h>

int
main(int argc, char *argv[])
{
    cl_int err = 0;

    cl_device_id device;
    cl_context context = 0;
    cl_command_queue queue = 0;

    IplImage *input = NULL, *tmp = NULL, *output = NULL;

    cl_image_format imageformat = {CL_BGRA, CL_UNSIGNED_INT8};
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1}; /* To update when we got image size */
    cl_mem climage_input, climage_pyramid;
    
    int scale = 0, maxscale = 0;

    struct timeval start, stop;

    /* OpenCL init, using our utils functions */
    clgp_maxflops_device(&device);

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

    /* Create output image */
    output = 
        cvCreateImage(
                cvSize(input->width*1.5, input->height),
                input->depth, 
                4);

    /* Convert input to an acceptable OpenCL format (we don't do {RGB,UINT8}) */
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
    climage_pyramid =
        clCreateImage2D(
                context,
                CL_MEM_READ_WRITE,
                &imageformat,
                input->width*1.5,
                input->height/(1<<scale),
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_pyramid\n");
        exit(1);
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
    gettimeofday(&start, NULL);
    clgp_pyramid(
            climage_pyramid, 
            climage_input, 
            input->width, 
            input->height, 
            maxscale);
    gettimeofday(&stop, NULL);
    printf(" - Done in %f ms\n", 
            (stop.tv_sec - start.tv_sec)*1000.f + (stop.tv_usec - start.tv_usec)/1000.f);


    /* Retrieve images */
    region[0] = input->width*1.5;
    region[1] = input->height;
    err = 
        clEnqueueReadImage(
                queue,
                climage_pyramid,
                CL_TRUE,
                origin,
                region,
                output->widthStep,
                0,
                output->imageData,
                0,
                NULL,
                NULL);
    clFinish(queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, 
                "Could not copy data on host (%i)\n", err);
        exit(1);
    }


    /* Release the clgp library */
    clgp_release();


    /* Release device ressources */
    clReleaseMemObject(climage_input);
    clReleaseMemObject(climage_pyramid);

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

