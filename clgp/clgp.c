#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "convolution.h"
#include "downscale.h"

#ifndef CLFLAGS
# define CLFLAGS "-cl-mad-enable -cl-fast-relaxed-math"
#endif

/* Global variables for the library */
/* The context the library will use, for now allow only one instance related
 * to one context. */
cl_context clgp_context; 
/* The command queue used by every clpg function */
cl_command_queue clgp_queue; 

/* Ressources for the downscaling */
extern const char clgp_downscale_kernel_src[];
cl_program clgp_downscale_program;
cl_kernel clgp_downscale_kernel;

/* Ressources for the convolution */
extern const char clgp_convolution_kernel_src[];
cl_program clgp_convolution_program;
cl_kernel clgp_convolution_kernel;

void clgp_release();

/* Create the command queue for clgp operation, build kernels, must be called
 * before any other function */
int
clgp_init(cl_context context, cl_command_queue queue)
{
    cl_int cl_err = 0;
    int clgp_err = 0;

    char *source = NULL;

#ifdef DEBUG
    char build_log[20000];
    cl_device_id device;
#endif

    clgp_context = 0;
    clgp_queue = 0;
    clgp_downscale_program = 0;
    clgp_downscale_kernel = 0;

    /* Build the programs, find the kernels... */
    /* Downscaling */
    source = (char *)clgp_downscale_kernel_src;
    clgp_downscale_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &cl_err);
    if (cl_err != CL_SUCCESS) {
        fprintf(stderr,
                "Could not create the clgp_downscale program\n");
        clgp_err = -1;
        goto end;
    }
    cl_err =
        clBuildProgram(clgp_downscale_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (cl_err != CL_SUCCESS) {
#ifdef DEBUG
        clGetContextInfo(
                context,
                CL_CONTEXT_DEVICES,
                1,
                &device,
                NULL);
        clGetProgramBuildInfo(
                clgp_downscale_program,
                device,
                CL_PROGRAM_BUILD_LOG,
                sizeof(build_log),
                build_log,
                NULL);
        fprintf(stderr, 
                "Could not build the clgp_downscale program\n%s\n", build_log);
#else
        fprintf(stderr,
                "Could not build the clgp_downscale program\n");
#endif
        clgp_err = -1;
        goto end;
    }
    clgp_downscale_kernel = 
        clCreateKernel(clgp_downscale_program, "downscale", &cl_err);
    if (cl_err != CL_SUCCESS) {
        fprintf(stderr, "Error: downscale kernel not found\n");
        clgp_err = -1;
        goto end;
    }
    /* Convolution */
    source = (char *)clgp_convolution_kernel_src;
    clgp_convolution_program =
        clCreateProgramWithSource(
                context,
                1,
                (char const **)&source,
                NULL,
                &cl_err);
    if (cl_err != CL_SUCCESS) {
        fprintf(stderr,
                "Could not create the clgp_convolution program\n");
        clgp_err = -1;
        goto end;
    }
    cl_err =
        clBuildProgram(clgp_convolution_program, 0, NULL, CLFLAGS, NULL, NULL);
    if (cl_err != CL_SUCCESS) {
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
                "Could not build the clgp_convolution program\n%s\n", build_log);
#else
        fprintf(stderr,
                "Could not build the clgp_convolution program\n");
#endif
        clgp_err = -1;
        goto end;
    }
    clgp_convolution_kernel = 
        clCreateKernel(clgp_convolution_program, "convolution", &cl_err);
    if (cl_err != CL_SUCCESS) {
        fprintf(stderr, "Error: convolution kernel not found\n");
        clgp_err = -1;
        goto end;
    }

    clgp_context = context;
    clgp_queue = queue;
end:
    if (clgp_err != 0) {
        clgp_release();
    }
    return clgp_err;
}

/* Release the ressources used by the library, this do NOT destroy the context 
 * which was passed from outside */
void
clgp_release()
{
    /* Wait for unfinished jobs */
    clFinish(clgp_queue);
    
    /* Free our program and kernels */
    if (clgp_downscale_kernel != 0) {
        clReleaseKernel(clgp_downscale_kernel);
    }
    if (clgp_downscale_program != 0) {
        clReleaseProgram(clgp_downscale_program);
    }
}

/* Util function to get the max scale for an image */
int
clgp_maxscale(int width, int height)
{
    /* 32x32 is the pratical min size of reduced image because we use 16x16
     * NDRange... To get around this limitation, we could trigger smaller 
     * sizes in downscale/convolution functions when width or height 
     * reach that limit (TODO) */ 
    return (int)log2f((float)((width > height) ? width : height)/32.f) + 1;
}

/* Build an array of images that are the different layers of the gaussian
 * pyramid */
int
clgp_pyramid(
        cl_mem *pyramid_images,
        cl_mem input_image,
        int width,
        int height,
        int maxscale)
{
    cl_image_format imageformat = {CL_BGRA, CL_UNSIGNED_INT8};
    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {width, height, 1};
    cl_mem tmp_image = 0;

    int scale = 0;

    cl_int cl_err;

    /* Allocate a temporary image to store reduced images before convolution */
    tmp_image =
        clCreateImage2D(
                clgp_context,
                CL_MEM_READ_WRITE,
                &imageformat,
                width,
                height,
                0,
                NULL,
                &cl_err);
    if (cl_err != CL_SUCCESS) {
        fprintf(stderr, "Could not allocate tmp_image\n");
    }

    /* Compute the pyramid */
    cl_err =
        clEnqueueCopyImage(
                clgp_queue,
                input_image,
                tmp_image,
                origin,
                origin,
                region,
                0,
                NULL,
                NULL);
    clFinish(clgp_queue);
    for (scale = 0; scale < maxscale; scale++) {
        clgp_downscale(
                tmp_image, 
                input_image, 
                width,
                height,
                scale);
        clgp_convolution(
                pyramid_images[scale], 
                tmp_image,
                width/(1<<scale),
                height/(1<<scale));
    }

    clReleaseMemObject(tmp_image);

    return 0;
}

