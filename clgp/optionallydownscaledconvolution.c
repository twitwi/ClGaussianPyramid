#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "error.h"

#define SCALE_ORIGIN_X(scale, width, height) \
    ((scale != 0)*width)

#define SCALE_ORIGIN_Y(scale, width, height) \
    ((scale >= 2) ? (int)((( (1.f - powf(0.5f, (float)scale)) / (1.f-0.5f) ) - 1.f)*(float)height) : 0) 

extern cl_int clgp_clerr;

extern cl_context clgp_context;
extern cl_command_queue clgp_queue;

extern cl_kernel clgp_downscaledconvolution_kernel;


int
clgp_optionallydownscaledconvolution(
        cl_mem output_image, 
        int out_x_offset,
        int out_y_offset,
        cl_mem input_image,
        int in_x_offset,
        int in_y_offset,
        int in_width,
        int in_height,
        int dowscaleOrNot)
{
    int width = in_width;
    int height = in_height;
    int scale = dowscaleOrNot ? 1 : 0;

    int err = 0;

    size_t local_work_size[2];
    size_t global_work_size[2];

    local_work_size[0] = (width >= 32) ? 16 : 8;
    local_work_size[1] = (height >= 32) ? 16 : 8;
    global_work_size[0] = ((width/(1<<scale)-1) / local_work_size[0] + 1)*local_work_size[0];
    global_work_size[1] = ((height/(1<<scale)-1) / local_work_size[1] + 1)*local_work_size[1];

    clSetKernelArg(clgp_downscaledconvolution_kernel, 0, sizeof(cl_mem), (void *)&output_image);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 1, sizeof(int), &out_x_offset);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 2, sizeof(int), &out_y_offset);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 3, sizeof(cl_mem), (void *)&input_image);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 4, sizeof(int), &in_x_offset);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 5, sizeof(int), &in_y_offset);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 6, sizeof(int), &scale);
    clFinish(clgp_queue);

    clgp_clerr = 
        clEnqueueNDRangeKernel(
                clgp_queue, 
                clgp_downscaledconvolution_kernel, 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);
    clFinish(clgp_queue);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not run the optionally downscaled convolution kernel\n");
        clgp_clerr = CLGP_CL_ERROR;
    }

    return err;
}

