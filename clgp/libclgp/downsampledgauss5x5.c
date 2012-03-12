/* Copyright (c) 2009-2012 Matthieu Volat and Remi Emonet. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <math.h>
#include <stdio.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#define DOWNSAMPLEDGAUSS5X5_ROWS 3
#define DOWNSAMPLEDGAUSS5X5_COLS 4


cl_int
clgpEnqueueDownsampledGauss5x5_cols(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem output_image, 
        cl_mem input_image,
        size_t width,
        size_t height)
{
    cl_int err = 0;

    size_t local_work_size[2];
    size_t global_work_size[2];

    local_work_size[0] = (width >= 32) ? 16 : width>>1;
    local_work_size[1] = (height >= 32) ? 16 : height>>1;
    global_work_size[0] = 
        ((width/2-1) / local_work_size[0] + 1)*local_work_size[0];
    global_work_size[1] = 
        ((height-1) / local_work_size[1] + 1)*local_work_size[1];

    clSetKernelArg(kernels[DOWNSAMPLEDGAUSS5X5_COLS], 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(kernels[DOWNSAMPLEDGAUSS5X5_COLS], 1, sizeof(cl_mem), &input_image);

    err = 
        clEnqueueNDRangeKernel(
                command_queue, 
                kernels[DOWNSAMPLEDGAUSS5X5_COLS], 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);
    if (err != CL_SUCCESS) {
#ifdef DEBUG 
        fprintf(stderr, "clgp: Could not run the downscaled convolution kernel\n");
#endif
        goto end;
    }

end:
    return err;
}

cl_int
clgpEnqueueDownsampledGauss5x5_rows(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem output_image, 
        cl_mem input_image,
        size_t width,
        size_t height)
{
    cl_int err = 0;

    size_t local_work_size[2];
    size_t global_work_size[2];

    local_work_size[0] = (width >= 32) ? 16 : width>>1;
    local_work_size[1] = (height >= 32) ? 16 : height>>1;
    global_work_size[0] = 
        ((width-1) / local_work_size[0] + 1)*local_work_size[0];
    global_work_size[1] = 
        ((height/2-1) / local_work_size[1] + 1)*local_work_size[1];

    clSetKernelArg(kernels[DOWNSAMPLEDGAUSS5X5_ROWS], 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(kernels[DOWNSAMPLEDGAUSS5X5_ROWS], 1, sizeof(cl_mem), &input_image);

    err = 
        clEnqueueNDRangeKernel(
                command_queue, 
                kernels[DOWNSAMPLEDGAUSS5X5_ROWS], 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);
    if (err != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: Could not run the downscaled convolution kernel\n");
#endif
        goto end;
    }

end:
    return err;
}

