
#include "GaussPyramid.h"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

#include <clgp.h>
#include <utils.h>
#include <cv.h>

/* Get the x origin of the level in the pyramid image */
static int LEVEL_ORIGIN_X(int level, int width, int height) {
    return ((level == 0) ? 0 : (int)((( (1.f - powf(0.5f, (float)(level>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width));
}
/* Get the y origin of the level in the pyramid image */
static int LEVEL_ORIGIN_Y(int level, int width, int height) {
    return ((level <= 2) ? 0 : (level & 0x1)*(height>>(level>>1)));
}
static double timeFrom(struct timeval start) {
    timeval stop;
    gettimeofday(&stop, NULL);
    double time = (stop.tv_sec-start.tv_sec)*1000.f + (stop.tv_usec-start.tv_usec)/1000.f;
    return time;
}
static double timeFromAndUpdate(struct timeval& start) {
    timeval stop;
    gettimeofday(&stop, NULL);
    double time = (stop.tv_sec-start.tv_sec)*1000.f + (stop.tv_usec-start.tv_usec)/1000.f;
    start = stop;
    return time;
}
#define SAFE_FREE(a) {if (a) free(a);}


GaussPyramid::GaussPyramid() {
    context = 0;
    queue = 0;
    cache.w = -1;
    cache.h = -1;
    cache.inRGBA = NULL;
    cache.outRGB = NULL;
    cache.outRGBA = NULL;
}

void GaussPyramid::initModule() {
    cl_int err = 0;
    cl_device_id device;
    struct timeval start;
    gettimeofday(&start, NULL);

    /* OpenCL init, using our utils functions */
    clgpMaxflopsDevice(&device);

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
    if (clgpInit(context, queue) != 0) {
        fprintf(stderr, "Could not init clgp library\n");
        exit(1);
    }
    printf(" * Init time: %f ms\n", timeFrom(start));

    SAFE_FREE(cache.inRGBA);
    SAFE_FREE(cache.outRGB);
    SAFE_FREE(cache.outRGBA);
}
void GaussPyramid::stopModule() {
    struct timeval start;
    gettimeofday(&start, NULL);
    /* Release the clgp library */
    clgpRelease();
    /* Release device ressources */
    clReleaseContext(context);
    clReleaseCommandQueue(queue);
    printf(" * Shutdown time: %f ms\n", timeFrom(start));
}

void GaussPyramid::input(IplImage *im) {
    IplImage *rgba = cvCreateImage(cvSize(im->width, im->height), IPL_DEPTH_8U, 4);
    cvCvtColor(im, rgba, CV_RGB2RGBA);
    inputRGBA(rgba->imageData, im->width, im->height);
    cvReleaseImage(&rgba);
}
void GaussPyramid::inputRGB(char* dataRGB, int w, int h) {
    char* dataRGBA = (char*) malloc(w*h*4);
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++) {
            dataRGBA[y*w*4 + x*4 + 0] = dataRGB[y*w*3 + x*3 + 0];
            dataRGBA[y*w*4 + x*4 + 1] = dataRGB[y*w*3 + x*3 + 1];
            dataRGBA[y*w*4 + x*4 + 2] = dataRGB[y*w*3 + x*3 + 2];
            dataRGBA[y*w*4 + x*4 + 3] = 255;
        }
    inputRGBA(dataRGBA, w, h);
    free(dataRGBA);
}
void GaussPyramid::outputBoth(char* dataRGBA, int wRef, int h) {
    emitNamedEvent("outputRGBA", dataRGBA, wRef, h);

    int w = 3*wRef;
    char* dataRGB = (char*) malloc(w*h*3);
    for(int y=0; y<h; y++)
        for(int x=0; x<w; x++) {
            dataRGB[y*w*3 + x*3 + 0] = dataRGBA[y*w*4 + x*4 + 0];
            dataRGB[y*w*3 + x*3 + 1] = dataRGBA[y*w*4 + x*4 + 1];
            dataRGB[y*w*3 + x*3 + 2] = dataRGBA[y*w*4 + x*4 + 2];
        }
    char* toSend = dataRGB;
    emitNamedEvent("output", toSend, w, h);
    free(dataRGB);
}

void GaussPyramid::inputRGBA(char* dataRGBA, int w, int h) {

    cl_int err = 0;
    unsigned char *input_data = NULL, *pyramid_data;
    unsigned int input_width = 0, pyramid_width = 0;
    unsigned int input_height = 0, pyramid_height = 0;
    unsigned int input_nbchannels = 0, pyramid_nbchannels = 0;

    cl_image_format imageformat = {CL_RGBA, CL_UNSIGNED_INT8};
    cl_mem input_climage, pyramid_climage[32];
    int maxlevel = 0, level = 0;
    
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    struct timeval start;
    struct timeval step;
    gettimeofday(&start, NULL);
    step = start;

    input_width = w;
    input_height = h;
    input_nbchannels = 4;
    input_data = (unsigned char*)dataRGBA;

    pyramid_width = 3 * input_width;
    pyramid_height = input_height;
    pyramid_nbchannels = input_nbchannels;
    pyramid_data = (unsigned char *)malloc(pyramid_width*pyramid_height*input_nbchannels*sizeof(char));

    printf("%d %d %d %lX\n", input_width, input_height, input_nbchannels, (long unsigned int) input_data);
    printf(" * local allocation time: %f ms\n", timeFromAndUpdate(step));

    /* Create buffers on device */
    input_climage =
        clgpCreateImage2D(
                CL_MEM_READ_ONLY,
                &imageformat,
                input_width,
                input_height,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate input_climage\n");
        exit(1);
    }

    maxlevel = clgpMaxlevel(input_width, input_height);
    for (level = 0; level < maxlevel; level++) {
        pyramid_climage[level] =
            clgpCreateImage2D(
                    CL_MEM_READ_WRITE,
                    &imageformat,
                    input_width >> (level>>1),
                    input_height >> (level>>1),
                    &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Could not allocate pyramid_climage[%d]\n", level);
            exit(1);
        }
    }

    printf(" * gpu allocation time: %f ms\n", timeFromAndUpdate(step));

    /* Send input image on device */
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
    printf(" * sending image to gpu time: %f ms\n", timeFromAndUpdate(step));


    /* At last, call our pyramid function */
    clgpBuildPyramid(
            pyramid_climage, 
            input_climage, 
            input_width, 
            input_height);
    clFinish(queue);
    printf(" * running pyramid: %f ms\n", timeFromAndUpdate(step));

    /* Retrieve images */
    for (level = 0; level < maxlevel; level++) {
        region[0] = input_width >> (level>>1);
        region[1] = input_height >> (level>>1);
        err = 
            clEnqueueReadImage(
                    queue,
                    pyramid_climage[level],
                    CL_TRUE,
                    origin,
                    region,
                    pyramid_width*pyramid_nbchannels*sizeof(char),
                    0,
                    (void *)((char *)((char *)pyramid_data + LEVEL_ORIGIN_Y(level, input_width, input_height)*pyramid_width*pyramid_nbchannels) + LEVEL_ORIGIN_X(level, input_width, input_height)*pyramid_nbchannels),
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
    printf(" * retrieving pyramid: %f ms\n", timeFromAndUpdate(step));
    printf(" * overall: %f ms\n", timeFrom(start));

    /* Free images */
    outputBoth((char*)pyramid_data, w, h);
    //free(input_data);
    free(pyramid_data);


    clReleaseMemObject(input_climage);
    for (level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climage[level]);
    }

}

