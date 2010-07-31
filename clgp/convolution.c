#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "error.h"

extern cl_int clgp_clerr;

extern cl_context clgp_context;
extern cl_command_queue clgp_queue;

extern cl_kernel clgp_convolution_rows_kernel;
extern cl_kernel clgp_convolution_cols_kernel;


int
clgpConvolution(
        cl_mem output_image, 
        int output_origin_x,
        int output_origin_y,
        cl_mem input_image,
        int input_origin_x,
        int input_origin_y,
        int width,
        int height)
{
    int err = 0;

    size_t local_work_size[2];
    size_t global_work_size[2];

    local_work_size[0] = (width >= 16) ? 16 : 8;
    local_work_size[1] = (height >= 16) ? 16 : 8;
    global_work_size[0] = 
        ((width-1) / local_work_size[0] + 1)*local_work_size[0];
    global_work_size[1] = 
        ((height-1) / local_work_size[1] + 1)*local_work_size[1];

    clSetKernelArg(clgp_convolution_rows_kernel, 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(clgp_convolution_rows_kernel, 1, sizeof(int), &output_origin_x);
    clSetKernelArg(clgp_convolution_rows_kernel, 2, sizeof(int), &output_origin_y);
    clSetKernelArg(clgp_convolution_rows_kernel, 3, sizeof(cl_mem), &input_image);
    clSetKernelArg(clgp_convolution_rows_kernel, 4, sizeof(int), &input_origin_x);
    clSetKernelArg(clgp_convolution_rows_kernel, 5, sizeof(int), &input_origin_y);

    clgp_clerr = 
        clEnqueueNDRangeKernel(
                clgp_queue, 
                clgp_convolution_rows_kernel, 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);
    clFinish(clgp_queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not run the convolution kernel\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    clSetKernelArg(clgp_convolution_cols_kernel, 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(clgp_convolution_cols_kernel, 1, sizeof(int), &output_origin_x);
    clSetKernelArg(clgp_convolution_cols_kernel, 2, sizeof(int), &output_origin_y);
    clSetKernelArg(clgp_convolution_cols_kernel, 3, sizeof(cl_mem), &input_image);
    clSetKernelArg(clgp_convolution_cols_kernel, 4, sizeof(int), &input_origin_x);
    clSetKernelArg(clgp_convolution_cols_kernel, 5, sizeof(int), &input_origin_y);

    clgp_clerr = 
        clEnqueueNDRangeKernel(
                clgp_queue, 
                clgp_convolution_cols_kernel, 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);
    clFinish(clgp_queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not run the convolution kernel\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

end:
    return err;
}

