#include <stdio.h>

#include <CL/cl.h>

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
/* ... */

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
        clBuildProgram(clgp_downscale_program, 0, NULL, "", NULL, NULL);
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
    /* ... */

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

