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
clgpEnqueueDownscaledGauss5x5(
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

    local_work_size[0] = (width >= 32) ? 16 : width>>1;
    local_work_size[1] = (height >= 32) ? 16 : height>>1;
    global_work_size[0] = 
        ((width/2-1) / local_work_size[0] + 1)*local_work_size[0];
    global_work_size[1] = 
        ((height/2-1) / local_work_size[1] + 1)*local_work_size[1];

    clSetKernelArg(kernels[DOWNSCALEDGAUSS5X5], 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(kernels[DOWNSCALEDGAUSS5X5], 1, sizeof(cl_mem), &input_image);

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
    if (clgp_clerr != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: downscaled5x5 kernel failure\n");
#endif
        err = CLGP_CL_ERROR;
        goto end;
    }

end:
    return err;
}

