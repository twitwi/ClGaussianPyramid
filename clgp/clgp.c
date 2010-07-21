#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "convolution.h"
#include "downscaledconvolution.h"
#include "error.h"

#ifndef CLFLAGS
# define CLFLAGS "-cl-mad-enable -cl-fast-relaxed-math"
#endif

#define SCALE_ORIGIN_X(scale, width, height) \
        (scale == 0) ? 0 : (int)((( (1.f - powf(0.5f, (float)(scale>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width)

#define SCALE_ORIGIN_Y(scale, width, height) \
        (scale <= 2) ? 0 : (scale & 0x1)*(height>>(scale>>1))

extern cl_int clgp_clerr;

/* Global variables for the library */
/* The context the library will use, for now allow only one instance related
 * to one context. */
cl_context clgp_context = 0; 
/* The command queue used by every clpg function */
cl_command_queue clgp_queue = 0; 

/* Ressources for the convolution */
extern const char clgp_convolution_kernel_src[];
cl_program clgp_convolution_program;
cl_kernel clgp_convolution_kernel;
/* Ressources for the downscaledconvolution */
extern const char clgp_downscaledconvolution_kernel_src[];
cl_program clgp_downscaledconvolution_program;
cl_kernel clgp_downscaledconvolution_kernel;

void clgp_release();

/* Create the command queue for clgp operation, build kernels, must be called
 * before any other function */
int
clgp_init(cl_context context, cl_command_queue queue)
{
    int err = 0;

    char *source = NULL;

#ifdef DEBUG
    char build_log[20000];
    cl_device_id device;
#endif

    clgp_context = 0;
    clgp_queue = 0;

    /* Build the programs, find the kernels... */
    /* Convolution */
    source = (char *)clgp_convolution_kernel_src;
    clgp_convolution_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr,
                "clgp: Could not create the clgp_convolution program\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(clgp_convolution_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetContextInfo(
                context,
                CL_CONTEXT_DEVICES,
                1,
                &device,
                NULL);
        clGetProgramBuildInfo(
                clgp_convolution_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: Could not build the clgp_convolution program\n%s\n", build_log);
#else
        fprintf(stderr,
                "clgp: Could not build the clgp_convolution program\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_convolution_kernel = 
        clCreateKernel(clgp_convolution_program, "convolution", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: convolution kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    /* Downscaled convolution */
    source = (char *)clgp_downscaledconvolution_kernel_src;
    clgp_downscaledconvolution_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr,
                "clgp: Could not create the clgp_downscaledconvolution program\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(clgp_downscaledconvolution_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetContextInfo(
                context,
                CL_CONTEXT_DEVICES,
                1,
                &device,
                NULL);
        clGetProgramBuildInfo(
                clgp_downscaledconvolution_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: Could not build the clgp_downscaledconvolution program\n%s\n", build_log);
#else
        fprintf(stderr,
                "clgp: Could not build the clgp_downscaledconvolution program\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_downscaledconvolution_kernel = 
        clCreateKernel(clgp_downscaledconvolution_program, "downscaledconvolution", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: downscaledconvolution kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Mark the context as used by our library */
    clgp_clerr = clRetainContext(context);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: could not retain context\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Finaly, set our internal pointers to the context and command queue */
    clgp_context = context;
    clgp_queue = queue;

end:
    if (err != 0) {
        clgp_release();
    }
    return err;
}

/* Release the ressources used by the library, this do NOT destroy the context 
 * which was passed from outside */
void
clgp_release()
{
    /* Wait for unfinished jobs */
    clFinish(clgp_queue);
    
    /* Free our program and kernels */
    if (clgp_convolution_kernel != 0) {
        clReleaseKernel(clgp_convolution_kernel);
    }
    if (clgp_convolution_program != 0) {
        clReleaseProgram(clgp_convolution_program);
    }
    if (clgp_downscaledconvolution_kernel != 0) {
        clReleaseKernel(clgp_downscaledconvolution_kernel);
    }
    if (clgp_downscaledconvolution_program != 0) {
        clReleaseProgram(clgp_downscaledconvolution_program);
    }

    /* Release the context -- the calling app stil has to do its own context
     * release */
    clReleaseContext(clgp_context);

    clgp_context = 0;
    clgp_queue = 0;
}

/* Util function to get the max scale for an image */
int
clgp_maxscale(int width, int height)
{
    /* 16x16 is the pratical min size of reduced image because of a strange
     * bug when trying to go for 8x8 */
    return (int)2.f*log2f((float)((width > height) ? width : height)/16.f) + 1;
}

/* Build an array of images that are the different layers of the gaussian
 * pyramid */
int
clgp_pyramid(
        cl_mem pyramid_image,
        cl_mem input_image,
        int width,
        int height)
{
    int err = 0;

    int maxscale = 0;
    int scale = 0;

    maxscale = clgp_maxscale(width, height);

    /* First iteration manualy */
    /* First half octave */
    err = 
        clgp_convolution(
                pyramid_image, 
                0,
                0,
                input_image,
                0,
                0,
                width,
                height);
    err = 
        clgp_convolution(
                pyramid_image, 
                0,
                0,
                pyramid_image,
                0,
                0,
                width,
                height);
    /* Second half-octave */
    err = 
        clgp_convolution(
                pyramid_image, 
                width,
                0,
                pyramid_image,
                0,
                0,
                width,
                height);
    err = 
        clgp_convolution(
                pyramid_image, 
                width,
                0,
                pyramid_image,
                width,
                0,
                width,
                height);

    /* Now go through the other octaves until we can't reduce */
    for (scale = 2; scale < maxscale; scale++) {
        /* First half octave : downscale + convolute */
        err = 
            clgp_downscaledconvolution(
                    pyramid_image,
                    SCALE_ORIGIN_X(scale, width, height),
                    SCALE_ORIGIN_Y(scale, width, height),
                    pyramid_image,
                    SCALE_ORIGIN_X(scale-1, width, height),
                    SCALE_ORIGIN_Y(scale-1, width, height),
                    width>>((scale-1)>>1),
                    height>>((scale-1)>>1));

        scale++;

        /* Second half octave : convolute + convolute */
        err = 
            clgp_convolution(
                    pyramid_image, 
                    SCALE_ORIGIN_X(scale, width, height),
                    SCALE_ORIGIN_Y(scale, width, height),
                    pyramid_image,
                    SCALE_ORIGIN_X(scale-1, width, height),
                    SCALE_ORIGIN_Y(scale-1, width, height),
                    width>>(scale>>1),
                    height>>(scale>>1));
        err = 
            clgp_convolution(
                    pyramid_image, 
                    SCALE_ORIGIN_X(scale, width, height),
                    SCALE_ORIGIN_Y(scale, width, height),
                    pyramid_image,
                    SCALE_ORIGIN_X(scale-1, width, height),
                    SCALE_ORIGIN_Y(scale-1, width, height),
                    width>>(scale>>1),
                    height>>(scale>>1));
    }

    return err;
}

