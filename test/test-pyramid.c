#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <CL/cl.h>

#include <cv.h>
#include <highgui.h>

#include <clgp.h>
#include <utils.h>

cl_int
read_clImage2D(
        cl_command_queue queue, 
        IplImage *iplImage,
        cl_mem clImage2D,
        int width,
        int height)
{
    cl_int err = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};

    err = 
        clEnqueueReadImage(
                queue,
                clImage2D,
                CL_TRUE,
                origin,
                region,
                iplImage->widthStep,
                0,
                iplImage->imageData,
                0,
                NULL,
                NULL);

    return err;
}

cl_int
write_clImage2D(
        cl_command_queue queue, 
        cl_mem clImage2D,
        IplImage *iplImage,
        int width,
        int height)
{
    cl_int err = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};

    err =
        clEnqueueWriteImage(
                queue,
                clImage2D,
                CL_TRUE,
                origin,
                region,
                iplImage->widthStep,
                0,
                iplImage->imageData,
                0,
                NULL,
                NULL);

    return err;
}

void
fill_pyramid(IplImage *ipl_pyramid, int width, int height)
{
    int scale = 0, maxscale = 0;

    maxscale = clgp_maxscale(width, height);

    for (scale = 2; scale < maxscale; scale++) {
        cvRectangle(
                ipl_pyramid,
                cvPoint(SCALE_ORIGIN_X(scale, width, height) + width/(1<<scale),
                        SCALE_ORIGIN_Y(scale, width, height)),
                cvPoint(width*1.5, height),
                cvScalar(0, 0, 0, 0),
                CV_FILLED,
                0,
                0);
    }
    if (scale >= 2) { /* Last part of the image */
        cvRectangle(
                ipl_pyramid,
                cvPoint(SCALE_ORIGIN_X(scale, width, height),
                        SCALE_ORIGIN_Y(scale, width, height)),
                cvPoint(width*1.5, height),
                cvScalar(0, 0, 0, 0),
                CV_FILLED,
                0,
                0);
    }
}

int
main(int argc, char *argv[])
{
    cl_int err = 0;

    cl_device_id device;
    cl_context context = 0;
    cl_command_queue queue = 0;

    IplImage *ipl_input = NULL, *ipl_tmp = NULL, *ipl_pyramid = NULL;

    cl_image_format imageformat = {CL_BGRA, CL_UNSIGNED_INT8};
    cl_mem climage_input, climage_pyramid;
    
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
    ipl_input = cvLoadImage(argv[1], CV_LOAD_IMAGE_COLOR);
    if (ipl_input == NULL) {
        fprintf(stderr, "Could not load file %s\n", argv[1]);
        exit(1);
    }

    /* Convert ipl_input to an acceptable OpenCL format (we don't do {RGB,UINT8}) */
    ipl_tmp = 
        cvCreateImage(
                cvSize(ipl_input->width, ipl_input->height), 
                ipl_input->depth, 
                4);
    cvCvtColor(ipl_input, ipl_tmp, CV_BGR2BGRA);
    cvReleaseImage(&ipl_input);
    ipl_input = ipl_tmp;
    ipl_tmp = NULL;

    /* Create ipl_pyramid image */
    ipl_pyramid = 
        cvCreateImage(
                cvSize(ipl_input->width*1.5, ipl_input->height),
                ipl_input->depth, 
                4);

    /* Create buffers on device */
    climage_input =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY,
                &imageformat,
                ipl_input->width,
                ipl_input->height,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_input\n");
        exit(1);
    }

    climage_pyramid =
        clCreateImage2D(
                context,
                CL_MEM_READ_WRITE,
                &imageformat,
                ipl_input->width*1.5,
                ipl_input->height,
                0,
                NULL,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate climage_pyramid\n");
        exit(1);
    }

    /* Send input image on device */
    err =
        write_clImage2D(
                queue,
                climage_input, 
                ipl_input, 
                ipl_input->width,  
                ipl_input->height);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not copy ipl_input data on device (%d)\n", err);
        exit(1);
    }


    /* At last, call our pyramid function */
    gettimeofday(&start, NULL);
    clgp_pyramid(
            climage_pyramid, 
            climage_input, 
            ipl_input->width, 
            ipl_input->height);
    gettimeofday(&stop, NULL);
    printf(" - Done in %f ms\n", 
            (stop.tv_sec - start.tv_sec)*1000.f + (stop.tv_usec - start.tv_usec)/1000.f);


    /* Retrieve images */
    err =
        read_clImage2D(
                queue,
                ipl_pyramid, 
                climage_pyramid, 
                ipl_pyramid->width,  
                ipl_pyramid->height);
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
    /* Fill outside of the pyramid with black, more displayable */
    fill_pyramid(ipl_pyramid, ipl_input->width, ipl_input->height);
    /* Display */
    cvNamedWindow("gaussian pyramid", CV_WINDOW_AUTOSIZE);
    cvShowImage("gaussian pyramid", ipl_pyramid);
    while (cvWaitKey(0) != 27) {}
    cvDestroyWindow("gaussian pyramid");

    cvReleaseImage(&ipl_input);
    cvReleaseImage(&ipl_pyramid);

    return 0;
}

