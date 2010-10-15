#include <stdio.h>

#include <CL/cl.h>

#include "error.h"

extern cl_int clgp_clerr;

extern cl_context clgp_context; 

int
clgpFirstDevice(cl_device_id *id)
{
    int err = 0;

    cl_platform_id platforms[1];
    cl_uint n_platforms = 0;

    cl_device_id devices[1];
    cl_uint n_devs = 0;

    /* Enumerate platforms, we take the first available */
    clgp_clerr =
        clGetPlatformIDs(
                1,
                platforms,
                &n_platforms);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not check for platforms\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Enumerate devices */
    clgp_clerr =
        clGetDeviceIDs(
                platforms[0],
                CL_DEVICE_TYPE_GPU,
                1,
                devices,
                &n_devs);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not check for devices\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    if (n_devs == 0) {
        fprintf(stderr, "clgp: No device available on this host\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    *id = devices[0];

end:
    return err;
}

int
clgpMaxflopsDevice(cl_device_id *id)
{
    int err = 0;

    cl_platform_id platforms[1];
    cl_uint n_platforms = 0;

    cl_device_id devices[16];
    cl_uint n_devs = 0;

    unsigned int maxflops = 0;

    int d = 0;
    cl_uint d_clock_freq = 0, d_compute_unit_nb = 0;
    unsigned int d_flops = 0;

    /* Enumerate platforms, we take the first available */
    clgp_clerr =
        clGetPlatformIDs(
                1,
                platforms,
                &n_platforms);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not check for platforms\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Enumerate devices */
    clgp_clerr =
        clGetDeviceIDs(
                platforms[0],
                CL_DEVICE_TYPE_GPU,
                16,
                devices,
                &n_devs);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not check for devices\n");
        err = CLGP_CL_ERROR;
        goto end;
    }
    if (n_devs == 0) {
        fprintf(stderr, "clgp: No device available on this host\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Check each device */
    for (d = 0; d < n_devs; d++) {
        /* Get max clock and compute unit number */
        clgp_clerr = 
            clGetDeviceInfo(
                    devices[d],
                    CL_DEVICE_MAX_CLOCK_FREQUENCY,
                    sizeof(cl_uint),
                    &d_clock_freq,
                    NULL);
        if (clgp_clerr != CL_SUCCESS) {
            fprintf(stderr, "clgp: Could not get device info\n");
            err = CLGP_CL_ERROR;
            goto end;
        }
        clgp_clerr = 
            clGetDeviceInfo(
                    devices[d],
                    CL_DEVICE_MAX_COMPUTE_UNITS,
                    sizeof(cl_uint),
                    &d_compute_unit_nb,
                    NULL);
        if (clgp_clerr != CL_SUCCESS) {
            fprintf(stderr, "clgp: Could not get device info\n");
            err = CLGP_CL_ERROR;
            goto end;
        }
        /* If better than current, take it */
        d_flops = d_clock_freq*d_compute_unit_nb;
        if (d_flops > maxflops) {
            *id = devices[d];
            maxflops = d_flops;
        }
    }

end:
    return err;
}

cl_mem
clgpCreateImage2D (
        cl_mem_flags flags, 
        const cl_image_format *image_format, 
        size_t image_width, 
        size_t image_height, 
        cl_int *errcode_ret)
{
    return 
        clCreateImage2D(
                clgp_context, 
                flags, 
                image_format, 
                image_width, 
                image_height, 
                0, 
                NULL, 
                errcode_ret);
}

