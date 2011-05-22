#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include "downsampledgauss5x5.h"
#include "downscaledgauss5x5.h"
#include "error.h"
#include "gauss9x9.h"

#ifndef CLFLAGS
# define CLFLAGS "-cl-mad-enable -cl-fast-relaxed-math"
#endif

extern cl_int clgp_clerr;

/* Global variables for the library */
extern const char clgp_downsampledgauss5x5_src[];
extern const char clgp_downscaledgauss5x5_src[];
extern const char clgp_gauss9x9_src[];

#define DOWNSCALEDGAUSS5X5 0
#define GAUSS9X9_ROWS 1
#define GAUSS9X9_COLS 2
#define DOWNSAMPLEDGAUSS5X5_ROWS 3
#define DOWNSAMPLEDGAUSS5X5_COLS 4

void 
clgpRelease(cl_context context, cl_kernel *kernels);

/* Register clgp in opencl context, must be called before everyelse function */
int
clgpInit(cl_context context, cl_kernel **kernelsptr)
{
    int err = 0;

    cl_device_id device = 0;
    cl_bool has_image = CL_FALSE;

    cl_program downsampledgauss5x5_program = NULL;
    cl_program downscaledgauss5x5_program = NULL;
    cl_program gauss9x9_program = NULL;

    cl_kernel *kernels = NULL;

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
#ifdef DEBUG
        fprintf(stderr, "clgp: no device associated\n");
#endif
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
#ifdef DEBUG
        fprintf(stderr, "clgp: cannot check device image support\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    if (has_image == CL_FALSE) {
#ifdef DEBUG
        fprintf(stderr, "clgp: no image support\n");
#endif
        err = CLGP_NO_IMAGE_SUPPORT;
        goto end;
    }

    /* Allocate kernels array */
    kernels = (cl_kernel *)malloc(8*sizeof(cl_kernel));
    if (kernels == NULL) {
        err = CLGP_ENOMEM;
        goto end;
    }
    memset(kernels, 0, 8*sizeof(cl_kernel));

    /* Build the programs, find the kernels... */
    /* Downsampled 5x5 gaussian blur */
    source = (char *)clgp_downsampledgauss5x5_src;
    downsampledgauss5x5_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr,
                "clgp: Could not create the clgpDownsampledGauss5x5 program\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(downsampledgauss5x5_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                downsampledgauss5x5_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: Could not build the clgpDownsampledGauss5x5 program\n%s\n", build_log);
#else
        fprintf(stderr,
                "clgp: Could not build the clgpDownsampledGauss5x5 program\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    (*kernels)[DOWNSAMPLEDGAUSS5X5_COLS] = 
        clCreateKernel(downsampledgauss5x5_program, "downsampledgauss5x5_cols", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: downsampledgauss5x5_cols kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    (*kernels)[DOWNSAMPLEDGAUSS5X5_ROWS] = 
        clCreateKernel(downsampledgauss5x5_program, "downsampledgauss5x5_rows", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: downsampledgauss5x5_rows kernel not found\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    /* Downscaled 5x5 gaussian blur */
    source = (char *)clgp_downscaledgauss5x5_src;
    downscaledgauss5x5_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr,
                "clgp: downscaledgauss5x5 program creation error\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(downscaledgauss5x5_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                downscaledgauss5x5_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: downscaledgauss5x5 program build error\n%s\n", 
                build_log);
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    kernels[DOWNSCALEDGAUSS5X5] = 
        clCreateKernel(
                downscaledgauss5x5_program, 
                "downscaledgauss5x5", 
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: downscaledgauss5x5 kernel not found\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    /* 9x9 gaussian blur */
    source = (char *)clgp_gauss9x9_src;
    gauss9x9_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr,
                "clgp: clgpGauss9x9 program creation error\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    clgp_clerr =
        clBuildProgram(gauss9x9_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        clGetProgramBuildInfo(
                gauss9x9_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "clgp: clgpGauss9x9 program build error\n%s\n", build_log);
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    kernels[GAUSS9X9_ROWS] = 
        clCreateKernel(gauss9x9_program, "gauss9x9_rows", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: gauss9x9_rows kernel not found\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }
    kernels[GAUSS9X9_COLS] = 
        clCreateKernel(gauss9x9_program, "gauss9x9_cols", &clgp_clerr);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: gauss9x9_cols kernel not found\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Mark the context as used by our library */
    clgp_clerr = clRetainContext(context);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: context retaining error\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }

end:
    if (err != 0) {
        clgpRelease(context, kernels);
    }

    *kernelsptr = kernels;

    return err;
}

/* Release the ressources created by the library */
void
clgpRelease(cl_context context, cl_kernel *kernels)
{
    cl_program downsampledgauss5x5_program = NULL;
    cl_program downscaledgauss5x5_program = NULL;
    cl_program gauss9x9_program = NULL;

    /* Retrieve program references from kernels */
    clgp_clerr =
        clGetKernelInfo(
                kernels[DOWNSAMPLEDGAUSS5X5_COLS],
                CL_KERNEL_PROGRAM,
                sizeof(cl_program),
                &downsampledgauss5x5_program,
                NULL);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: could not access downsampledgauss5x5 program\n");
        return;
    }
    clgp_clerr =
        clGetKernelInfo(
                kernels[DOWNSCALEDGAUSS5X5],
                CL_KERNEL_PROGRAM,
                sizeof(cl_program),
                &downscaledgauss5x5_program,
                NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: downscaledgauss5x5 program error\n");
#endif
        return;
    }
    clgp_clerr =
        clGetKernelInfo(
                kernels[GAUSS9X9_COLS],
                CL_KERNEL_PROGRAM,
                sizeof(cl_program),
                &gauss9x9_program,
                NULL);
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: gauss9x9 program error\n");
#endif
        return;
    }
    /* Free our program and kernels */
    if (kernels[DOWNSAMPLEDGAUSS5X5_COLS] != NULL) {
        clReleaseKernel(kernels[DOWNSAMPLEDGAUSS5X5_COLS]);
    }
    if (kernels[DOWNSAMPLEDGAUSS5X5_ROWS] != NULL) {
        clReleaseKernel(kernels[DOWNSAMPLEDGAUSS5X5_ROWS]);
    }
    if (downsampledgauss5x5_program != NULL) {
        clReleaseProgram(downsampledgauss5x5_program);
    }
    if (kernels[DOWNSCALEDGAUSS5X5] != NULL) {
        clReleaseKernel(kernels[DOWNSCALEDGAUSS5X5]);
    }
    if (downscaledgauss5x5_program != NULL) {
        clReleaseProgram(downscaledgauss5x5_program);
    }
    if (kernels[GAUSS9X9_ROWS] != NULL) {
        clReleaseKernel(kernels[GAUSS9X9_ROWS]);
    }
    if (kernels[GAUSS9X9_COLS] != NULL) {
        clReleaseKernel(kernels[GAUSS9X9_COLS]);
    }
    if (gauss9x9_program != NULL) {
        clReleaseProgram(gauss9x9_program);
    }
    free(kernels);

    /* Release the context from *our* library */
    clReleaseContext(context);
}

/* Util function to get the max level for an image */
int
clgpMaxlevel(size_t width, size_t height)
{
    /* Go until the last reduction possible */
    return (int)log2f((float)((width > height) ? width : height)) + 1;
}

/* Build an array of images that are the different layers of the classic 
 * gaussian pyramid */
int
clgpBuildPyramid(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        int maxlevel)
{
    int err = 0;

    size_t width = 0;
    size_t height = 0;
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};
    cl_image_format input_format;
    int level = 0;

    clGetImageInfo(input_image, CL_IMAGE_WIDTH, sizeof(size_t), &width, NULL);
    clGetImageInfo(input_image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, NULL);
    region[0] = width;
    region[1] = height;

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

    /* First iteration -- just copy the image */
    err = 
        clEnqueueCopyImage(
                command_queue, 
                input_image, 
                pyramid_image[0], 
                origin, 
                origin, 
                region, 
                0, 
                NULL, 
                NULL);
    if (err != 0) {
        goto end;
    }
    /* Now go through the other levels until we can't reduce */
    for (level = 1; level < maxlevel; level++) {
        /* 5x5 blur + downscale */
        err = 
            clgpDownscaledGauss5x5(
                    command_queue,
                    kernels,
                    pyramid_image[level],
                    pyramid_image[level-1],
                    width>>(level-1),
                    height>>(level-1));
        if (err != 0) {
            goto end;
        }
    }

end:
    return err;
}

/* Util function to get the max level for an image */
int
clgpMaxlevelHalfOctave(size_t width, size_t height)
{
    /* Go until the last reduction possible */
    return 2*clgpMaxlevel(width, height);
}

/* Build an array of images that are the different layers of the half-octave
 * gaussian pyramid */
int
clgpBuildPyramidHalfOctave(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        int maxlevel)
{
    int err = 0;

    size_t width = 0;
    size_t height = 0;
    cl_image_format input_format;
    int level = 0;

    clGetImageInfo(input_image, CL_IMAGE_WIDTH, sizeof(size_t), &width, NULL);
    clGetImageInfo(input_image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, NULL);

    clGetImageInfo(
            input_image, 
            CL_IMAGE_FORMAT, 
            sizeof(cl_image_format),
            &input_format,
            NULL);
    if (input_format.image_channel_data_type != CL_UNSIGNED_INT8) {
#ifdef DEBUG
        fprintf(stderr, "clgp: wrong format\n");
#endif
        goto end;
    }

    /* First iteration manualy */
    /* First half octave */
    err = 
        clgpGauss9x9(
                command_queue,
                kernels,
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
                kernels,
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
                    kernels,
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
                    kernels,
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

/* Build an array of images that are the different layers of the half-octave 
 * gaussian pyramid with sqrt2 layout */
int
clgpBuildPyramidSqrt2(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        int maxlevel)
{
    int err = 0;

    size_t width = 0;
    size_t height = 0;
    cl_image_format input_format;
    int level = 0;

    clGetImageInfo(input_image, CL_IMAGE_WIDTH, sizeof(size_t), &width, NULL);
    clGetImageInfo(input_image, CL_IMAGE_HEIGHT, sizeof(size_t), &height, NULL);

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
                kernels,
                pyramid_image[0], 
                input_image,
                width,
                height);
    if (err != 0) {
        goto end;
    }
    /* Second half octave */
    err =
        clgpDownsampledGauss5x5_cols(
                command_queue,
                kernels,
                pyramid_image[1], 
                pyramid_image[0], 
                width,
                height);
    if (err != 0) {
        goto end;
    }
    /* Now go through the other octaves until we can't reduce */
    for (level = 2; level < maxlevel; level++) {
        if (level & 0x1) {
            /* Odd level : downsampled 5x5 blur on rows */
            err =
                clgpDownsampledGauss5x5_cols(
                        command_queue,
                        kernels,
                        pyramid_image[level], 
                        pyramid_image[level-1], 
                        width >> (level>>1),
                        height >> ((level-1)>>1));
            if (err != 0) {
                goto end;
            }
        }
        else {
            /* Even level: downsampled 5x5 blur on cols */
            err =
                clgpDownsampledGauss5x5_rows(
                        command_queue,
                        kernels,
                        pyramid_image[level], 
                        pyramid_image[level-1], 
                        width >> (level>>1),
                        height >> ((level-1)>>1));
            if (err != 0) {
                goto end;
            }
        }
    }

end:
    return err;
}

