#include <stdio.h>

#include <CL/cl.h>

extern cl_context clgp_context;
extern cl_command_queue clgp_queue;

extern cl_kernel clgp_downscaledconvolution_kernel;


void
clgp_downscaledconvolution(
        cl_mem convoluted_image, 
        cl_mem input_image,
        int width,
        int height,
        int scale)
{
    size_t local_work_size[2];
    size_t global_work_size[2];

    local_work_size[0] = 16;
    local_work_size[1] = 16;
    global_work_size[0] = ((width/(1<<scale)-1) / local_work_size[0] + 1)*16;
    global_work_size[1] = ((height/(1<<scale)-1) / local_work_size[1] + 1)*16;

    cl_int err = CL_SUCCESS;

    clSetKernelArg(clgp_downscaledconvolution_kernel, 0, sizeof(cl_mem), (void *)&convoluted_image);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 1, sizeof(cl_mem), (void *)&input_image);
    clSetKernelArg(clgp_downscaledconvolution_kernel, 2, sizeof(int), &scale);
    clFinish(clgp_queue);

    err = 
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
        fprintf(stderr, "Could not run the downscaled convolution kernel on device\n");
    }
}

