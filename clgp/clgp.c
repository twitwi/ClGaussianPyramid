#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "gauss9x9.h"
#include "downscaledgauss5x5.h"
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

/* Ressources for the downscaledgauss5x5 */
extern const char clgp_downscaledgauss5x5_kernel_src[];
cl_program clgpDownscaledGauss5x5_program;
cl_kernel clgp_downscaledgauss5x5_kernel;
/* Ressources for the gaussian 9x9 filtering */
extern const char clgp_gauss9x9_kernel_src[];
cl_program clgpGauss9x9_program;
cl_kernel clgp_gauss9x9_rows_kernel;
cl_kernel clgp_gauss9x9_cols_kernel;

void clgpRelease();

/* Create the command queue for clgp operation, build kernels, must be called
 * before any other function */
int
clgpInit(cl_context context, cl_command_queue queue)
{
    int err = 0;

    cl_device_id device = 0;
    cl_bool has_image = CL_FALSE;

    char *source = NULL;

#ifdef DEBUG
    char build_log[20000];
#endif

    clgp_context = 0;
    clgp_queue = 0;

    /* Check if device support images */
    clgp_clerr = 
        clGetContextInfo(
                context,
                CL_CONTEXT_DEVICES,
                sizeof(cl_device_id),
                &device,
                NULL);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: could not access context's device\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    clgp_clerr = 
        clGetDeviceInfo(
                device,
                CL_DEVICE_IMAGE_SUPPORT,
                sizeof(cl_bool),
                &has_image,
                NULL);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: could not check device image support\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    if (has_image == CL_FALSE) {
        fprintf(stderr, "clgp: device do not have image support\n");
        err = CLGP_NO_IMAGE_SUPPORT;
        goto end;
    }

    /* Build the programs, find the kernels... */
    /* Downscaled convolution */
    source = (char *)clgp_downscaledgauss5x5_kernel_src;
    clgpDownscaledGauss5x5_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr,
                "clgp: Could not create the clgpDownscaledGauss5x5 program\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(clgpDownscaledGauss5x5_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                clgpDownscaledGauss5x5_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: Could not build the clgpDownscaledGauss5x5 program\n%s\n", build_log);
#else
        fprintf(stderr,
                "clgp: Could not build the clgpDownscaledGauss5x5 program\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_downscaledgauss5x5_kernel = 
        clCreateKernel(clgpDownscaledGauss5x5_program, "downscaledgauss5x5", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: downscaledgauss5x5 kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    /* 9x9 gaussian filtering */
    source = (char *)clgp_gauss9x9_kernel_src;
    clgpGauss9x9_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr,
                "clgp: Could not create the clgpGauss9x9 program\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(clgpGauss9x9_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                clgpGauss9x9_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: Could not build the clgpGauss9x9 program\n%s\n", build_log);
#else
        fprintf(stderr,
                "clgp: Could not build the clgpGauss9x9 program\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_gauss9x9_rows_kernel = 
        clCreateKernel(clgpGauss9x9_program, "gauss9x9_rows", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: gauss9x9_rows kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_gauss9x9_cols_kernel = 
        clCreateKernel(clgpGauss9x9_program, "gauss9x9_cols", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: gauss9x9_cols kernel not found\n");
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
    clgp_context = context;

    /* Mark the command queue as used by our library */
    clgp_clerr = clRetainCommandQueue(queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: could not retain command queue\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_queue = queue;

end:
    if (err != 0) {
        clgpRelease();
    }
    return err;
}

/* Release the ressources created by the library */
void
clgpRelease()
{
    /* Wait for unfinished jobs */
    clFinish(clgp_queue);
    
    /* Free our program and kernels */
    if (clgp_gauss9x9_rows_kernel != 0) {
        clReleaseKernel(clgp_gauss9x9_rows_kernel);
    }
    if (clgp_gauss9x9_cols_kernel != 0) {
        clReleaseKernel(clgp_gauss9x9_cols_kernel);
    }
    if (clgpGauss9x9_program != 0) {
        clReleaseProgram(clgpGauss9x9_program);
    }
    if (clgp_downscaledgauss5x5_kernel != 0) {
        clReleaseKernel(clgp_downscaledgauss5x5_kernel);
    }
    if (clgpDownscaledGauss5x5_program != 0) {
        clReleaseProgram(clgpDownscaledGauss5x5_program);
    }

    /* Release the context from *our* library */
	if (clgp_context != 0) {
		clReleaseContext(clgp_context);
		clgp_context = 0;
	}

    /* Release the command queue from *our* library */
	if (clgp_context != 0) {
		clReleaseCommandQueue(clgp_queue);
		clgp_queue = 0;
	}
}

/* Util function to get the max level for an image */
int
clgpMaxlevel(int width, int height)
{
    /* 16x16 is the pratical min size for a double 5x5 filtering */
    return 2*((int)log2f((float)((width > height) ? width : height)/16.f) + 1);
}

/* Build an array of images that are the different layers of the gaussian
 * pyramid */
int
clgpBuildPyramid(
        cl_mem pyramid_image[],
        cl_mem input_image,
        int width,
        int height)
{
    int err = 0;

    int maxlevel = 0;
    int level = 0;

    maxlevel = clgpMaxlevel(width, height);

    /* First iteration manualy */
    /* First half octave */
    err = 
        clgpGauss9x9(
                pyramid_image[0], 
                input_image,
                width,
                height);
    /* Second half-octave */
    err = 
        clgpGauss9x9(
                pyramid_image[1], 
                pyramid_image[0],
                width,
                height);

    /* Now go through the other octaves until we can't reduce */
    for (level = 2; level < maxlevel; level++) {
        /* First half octave : downlevel + convolute */
        err = 
            clgpDownscaledGauss5x5(
                    pyramid_image[level],
                    pyramid_image[level-1],
                    width>>((level-1)>>1),
                    height>>((level-1)>>1));

        level++;

        /* Second half octave : convolute + convolute */
        err = 
            clgpGauss9x9(
                    pyramid_image[level], 
                    pyramid_image[level-1],
                    width>>(level>>1),
                    height>>(level>>1));
    }

    return err;
}

