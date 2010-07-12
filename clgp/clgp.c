#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "downscaledconvolution.h"
#include "error.h"

#ifndef CLFLAGS
# define CLFLAGS "-cl-mad-enable -cl-fast-relaxed-math"
#endif

extern cl_int clgp_clerr;

/* Global variables for the library */
/* The context the library will use, for now allow only one instance related
 * to one context. */
cl_context clgp_context; 
/* The command queue used by every clpg function */
cl_command_queue clgp_queue; 

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
}

/* Util function to get the max scale for an image */
int
clgp_maxscale(int width, int height)
{
    /* 32x32 is the pratical min size of reduced image because we use 16x16
     * NDRange... To get around this limitation, we could trigger smaller 
     * sizes downscaled convolution functions when width or height 
     * reach that limit (TODO) */ 
    return (int)log2f((float)((width > height) ? width : height)/16.f) + 1;
}

/* Build an array of images that are the different layers of the gaussian
 * pyramid */
int
clgp_pyramid(
        cl_mem pyramid_image,
        cl_mem input_image,
        int width,
        int height,
        int maxscale)
{
    int err = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};

    int scale = 0;

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

