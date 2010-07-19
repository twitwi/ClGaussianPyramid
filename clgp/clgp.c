#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "downscaledconvolution.h"
#include "optionallydownscaledconvolution.h"
#include "error.h"

#ifndef CLFLAGS
# define CLFLAGS "-cl-mad-enable -cl-fast-relaxed-math"
#endif

extern cl_int clgp_clerr;

/* Global variables for the library */
/* The context the library will use, for now allow only one instance related
 * to one context. */
cl_context clgp_context = 0; 
/* The command queue used by every clpg function */
cl_command_queue clgp_queue = 0; 

/* Ressources for the downscaledconvolution */
extern const char clgp_downscaledconvolution_kernel_src[];
extern const char clgp_optionallydownscaledconvolution_kernel_src[];
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
    /* Downscaled convolution */
    source = (char *)clgp_optionallydownscaledconvolution_kernel_src;
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
        clCreateKernel(clgp_downscaledconvolution_program, "optionallydownscaledconvolution", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: optionallydownscaledconvolution kernel not found\n");
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
    return (int)log2f((float)((width > height) ? width : height)/16.f) + 1;
}

/* Build an array of images that are the different layers of the gaussian
 * pyramid */
int
clgp_pyramid_old(
        cl_mem pyramid_image,
        cl_mem input_image,
        int width,
        int height)
{
    int err = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};

    int maxscale = 0;
    int scale = 0;

    maxscale = clgp_maxscale(width, height);

    /* Compute the pyramid */
    for (scale = 0; scale < maxscale; scale++) {
        err = 
            clgp_downscaledconvolution(
                    pyramid_image, 
                    input_image,
                    width,
                    height,
                    scale);
        if (err != 0) {
            break;
        }
    }

    return err;
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

    size_t origin[2] = {0, 0};
    size_t size[2] = {width, height};

    int maxscale = 0;
    int scale = 0;

    maxscale = clgp_maxscale(width, height)*2;
    /* compute the first filtered level */
    err = 
        clgp_optionallydownscaledconvolution(
                    pyramid_image, 
                    origin[0],
                    origin[1],
                    input_image,
                    0,
                    0,
                    size[0],
                    size[1],
                    0);
    if (err != 0) {
        return err;
    }

    /* Compute the rest of the pyramid */
    for (scale = 1; scale < maxscale; scale++) {
        fprintf(stderr, "SCALE %d %d %d\n", scale, origin[0], origin[1]);
        int isDownscaling = scale&1;
        size_t newOrigin[2] = {
            origin[0] + isDownscaling * size[0],
            (1-isDownscaling) * size[1],
        };
        if (1 && isDownscaling) {
            err = 
            clgp_optionallydownscaledconvolution(
                    pyramid_image, 
                    newOrigin[0] + size[0],
                    newOrigin[1],
                    pyramid_image,
                    origin[0],
                    origin[1],
                    size[0],
                    size[1],
                    0);
            if (err != 0) {
                break;
            }
            err = 
            clgp_optionallydownscaledconvolution(
                    pyramid_image, 
                    newOrigin[0],
                    newOrigin[1],
                    pyramid_image,
                    newOrigin[0] + size[0],
                    newOrigin[1],
                    size[0],
                    size[1],
                    isDownscaling);
            if (err != 0) {
                break;
            }
        } else {
            err = 
            clgp_optionallydownscaledconvolution(
                    pyramid_image, 
                    newOrigin[0],
                    newOrigin[1],
                    pyramid_image,
                    origin[0],
                    origin[1],
                    size[0],
                    size[1],
                    isDownscaling);
            if (err != 0) {
                break;
            }
        }
        origin[0] = newOrigin[0];
        origin[1] = newOrigin[1];
        if (isDownscaling) {
            size[0] >>= 1;
            size[1] >>= 1;
        }
    }

    return err;
}
