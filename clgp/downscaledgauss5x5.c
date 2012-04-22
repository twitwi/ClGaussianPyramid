#include <math.h>
#include <stdio.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include "error.h"

extern cl_int clgp_clerr;

#define DOWNSCALEDGAUSS5X5 0


int
clgpDownscaledGauss5x5(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem output_image, 
        cl_mem input_image,
        size_t width,
        size_t height)
{
    int err = 0;

    size_t local_work_size[2];
    size_t global_work_size[2];

#ifdef DEBUG /* Supposely already done by higher-level functions */
    cl_image_format input_format;
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
#endif

    local_work_size[0] = (width >= 32) ? 16 : width>>1;
    local_work_size[1] = (height >= 32) ? 16 : height>>1;
    global_work_size[0] = 
        ((width/2-1) / local_work_size[0] + 1)*local_work_size[0];
    global_work_size[1] = 
        ((height/2-1) / local_work_size[1] + 1)*local_work_size[1];

    clSetKernelArg(kernels[DOWNSCALEDGAUSS5X5], 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(kernels[DOWNSCALEDGAUSS5X5], 1, sizeof(cl_mem), &input_image);
    clFinish(command_queue);

    clgp_clerr = 
        clEnqueueNDRangeKernel(
                command_queue, 
                kernels[DOWNSCALEDGAUSS5X5], 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);

#ifdef DEBUG /* Systematicaly checking kernel execution is very costly */
    clFinish(command_queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not run the downscaled convolution kernel\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
#endif

end:
    return err;
}

