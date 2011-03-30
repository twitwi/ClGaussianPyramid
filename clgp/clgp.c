#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "downscaledgauss5x5.h"
#include "error.h"
#include "gauss9x9.h"

#ifndef CLFLAGS
# define CLFLAGS "-cl-mad-enable -cl-fast-relaxed-math"
#endif

extern cl_int clgp_clerr;

/* Global variables for the library */
/* Ressources for the downscaled gaussian 5x5 filtering */
extern const char clgp_downscaledgauss5x5_kernel_src[];
cl_program clgp_downscaledgauss5x5_program;
cl_kernel clgp_downscaledgauss5x5_kernel;
/* Ressources for the gaussian 9x9 filtering */
extern const char clgp_gauss9x9_kernel_src[];
cl_program clgp_gauss9x9_program;
cl_kernel clgp_gauss9x9_rows_kernel;
cl_kernel clgp_gauss9x9_cols_kernel;

void clgpRelease(cl_context context, cl_command_queue command_queue);

/* Create the command queue for clgp operation, build kernels, must be called
 * before any other function */
int
clgpInit(cl_context context, cl_command_queue command_queue)
{
    int err = 0;

    cl_device_id device = 0;
    cl_bool has_image = CL_FALSE;

    char *source = NULL;

#ifdef DEBUG
    char build_log[20000];
#endif

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
    /* Downscaled 5x5 gaussian blur */
    source = (char *)clgp_downscaledgauss5x5_kernel_src;
    clgp_downscaledgauss5x5_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr,
                "clgp: Could not create the clgp_downscaledgauss5x5_program program\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(clgp_downscaledgauss5x5_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                clgp_downscaledgauss5x5_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: Could not build the clgp_downscaledgauss5x5_program program\n%s\n", build_log);
#else
        fprintf(stderr,
                "clgp: Could not build the clgp_downscaledgauss5x5_program program\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_downscaledgauss5x5_kernel = 
        clCreateKernel(clgp_downscaledgauss5x5_program, "downscaledgauss5x5", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: downscaledgauss5x5 kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    /* 9x9 gaussian blur */
    source = (char *)clgp_gauss9x9_kernel_src;
    clgp_gauss9x9_program =
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
        clBuildProgram(clgp_gauss9x9_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                clgp_gauss9x9_program,
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
        clCreateKernel(clgp_gauss9x9_program, "gauss9x9_rows", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: gauss9x9_rows kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_gauss9x9_cols_kernel = 
        clCreateKernel(clgp_gauss9x9_program, "gauss9x9_cols", &clgp_clerr);
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

    /* Mark the command queue as used by our library */
    clgp_clerr = clRetainCommandQueue(command_queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: could not retain command queue\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

end:
    if (err != 0) {
        clgpRelease(context, command_queue);
    }
    return err;
}

/* Release the ressources created by the library */
void
clgpRelease(cl_context context, cl_command_queue command_queue)
{
    /* Free our program and kernels */
    if (clgp_downscaledgauss5x5_kernel != 0) {
        clReleaseKernel(clgp_downscaledgauss5x5_kernel);
    }
    if (clgp_downscaledgauss5x5_program != 0) {
        clReleaseProgram(clgp_downscaledgauss5x5_program);
    }
    if (clgp_gauss9x9_rows_kernel != 0) {
        clReleaseKernel(clgp_gauss9x9_rows_kernel);
    }
    if (clgp_gauss9x9_cols_kernel != 0) {
        clReleaseKernel(clgp_gauss9x9_cols_kernel);
    }
    if (clgp_gauss9x9_program != 0) {
        clReleaseProgram(clgp_gauss9x9_program);
    }

    /* Release the context from *our* library */
    clReleaseContext(context);

    /* Release the command queue from *our* library */
    clReleaseCommandQueue(command_queue);
}

/* Util function to get the max level for an image */
int
clgpMaxlevel(size_t width, size_t height)
{
    /* Go until the last reduction possible */
    return (int)log2f((float)((width > height) ? width : height));
}

/* Build an array of images that are the different layers of the half-octave
 * gaussian pyramid */
int
clgpBuildPyramidHalfOctave(
        cl_command_queue command_queue,
        cl_mem pyramid_image[],
        cl_mem input_image)
{
    int err = 0;

    size_t width = 0;
    size_t height = 0;
    cl_image_format input_format;
    int maxlevel = 0;
    int level = 0;

    clGetImageInfo(input_image, CL_IMAGE_WIDTH, sizeof(size_t), &width, NULL);
    clGetImageInfo(input_image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, NULL);
    maxlevel = clgpMaxlevel(width, height)*2;

    clGetImageInfo(
            input_image, 
            CL_IMAGE_FORMAT, 
            sizeof(cl_image_format),
            &input_format,
            NULL);
    if (input_format.image_channel_data_type != CL_UNSIGNED_INT8) {
        fprintf(stderr, "clgp: wrong format\n");
        goto end;
    }

    /* First iteration manualy */
    /* First half octave */
    err = 
        clgpGauss9x9(
                command_queue,
                pyramid_image[0], 
                input_image,
                width,
                height);
    if (err != 0) {
        goto end;
    }
    /* Second half-octave */
    err = 
        clgpGauss9x9(
                command_queue,
                pyramid_image[1], 
                pyramid_image[0],
                width,
                height);
    if (err != 0) {
        goto end;
    }
    /* Now go through the other octaves until we can't reduce */
    for (level = 2; level < maxlevel; level++) {
        /* First half octave : 5x5 blur + downscale */
        err = 
            clgpDownscaledGauss5x5(
                    command_queue,
                    pyramid_image[level],
                    pyramid_image[level-1],
                    width>>((level-1)>>1),
                    height>>((level-1)>>1));
        if (err != 0) {
            goto end;
        }

        level++;

        /* Second half octave : 9x9 blur */
        err = 
            clgpGauss9x9(
                    command_queue,
                    pyramid_image[level], 
                    pyramid_image[level-1],
                    width>>(level>>1),
                    height>>(level>>1));
        if (err != 0) {
            goto end;
        }
    }

end:
    return err;
}

