#include <math.h>
#include <stdio.h>

#include <CL/cl.h>

#include "error.h"

extern cl_int clgp_clerr;

extern cl_context clgp_context;
extern cl_command_queue clgp_queue;

extern cl_kernel clgp_gauss9x9_rows_kernel;
extern cl_kernel clgp_gauss9x9_cols_kernel;


int
clgpGauss9x9(
        cl_mem output_image, 
        cl_mem input_image,
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

    clSetKernelArg(clgp_gauss9x9_rows_kernel, 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(clgp_gauss9x9_rows_kernel, 1, sizeof(cl_mem), &input_image);

    clgp_clerr = 
        clEnqueueNDRangeKernel(
                clgp_queue, 
                clgp_gauss9x9_rows_kernel, 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);

#ifdef DEBUG /* Systematicaly checking kernel execution is very costly */
    clFinish(clgp_queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not run the gauss9x9_rows kernel\n");
        printf("%d\n", clgp_clerr);
        err = CLGP_CL_ERROR;
        goto end;
    }
#endif

    clSetKernelArg(clgp_gauss9x9_cols_kernel, 0, sizeof(cl_mem), &output_image);
    clSetKernelArg(clgp_gauss9x9_cols_kernel, 1, sizeof(cl_mem), &output_image);

    clgp_clerr = 
        clEnqueueNDRangeKernel(
                clgp_queue, 
                clgp_gauss9x9_cols_kernel, 
                2, 
                NULL,
                &global_work_size[0], 
                &local_work_size[0],
                0, 
                NULL, 
                NULL);

#ifdef DEBUG /* Systematicaly checking kernel execution is very costly */
    clFinish(clgp_queue);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not run the gauss9x9_cols kernel\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
#endif

end:
    return err;
}

