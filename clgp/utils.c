#include <stdio.h>

#ifndef __APPLE__
# include <CL/cl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include "error.h"

extern cl_int clgp_clerr;

extern cl_context clgp_context; 

static int
clgpFirstDevice(cl_device_id *id, cl_device_type device_type)
{
    int err = 0;

    cl_platform_id platforms[8];
    cl_uint n_platforms = 0;

    cl_device_id devices[16];
    cl_uint n_devs = 0;

    int p = 0;
    
    *id = NULL;

    /* Enumerate platforms, we take the first available */
    clgp_clerr =
        clGetPlatformIDs(
                8,
                platforms,
                &n_platforms);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not check for platforms\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    /* Enumerate devices */
    for (p = 0; p < n_platforms; p++) {
        clgp_clerr =
            clGetDeviceIDs(
                    platforms[p],
                    device_type,
                    16,
                    devices,
                    &n_devs);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "clgp: Could not check for devices\n");
            err = CLGP_CL_ERROR;
            goto end;
        }
        if (n_devs > 0) {
            *id = devices[0];
            break;
        }
    }

end:
    return err;
}

int
clgpFirstGPU(cl_device_id *id)
{
    return clgpFirstDevice(id, CL_DEVICE_TYPE_GPU);
}

int
clgpMaxflopsGPU(cl_device_id *id)
{
    int err = 0;

    cl_platform_id platforms[8];
    cl_uint n_platforms = 0;

    cl_device_id devices[16];
    cl_uint n_devs = 0;

    unsigned int maxflops = 0;

    int d = 0;
    cl_uint d_clock_freq = 0, d_compute_unit_nb = 0;
    unsigned int d_flops = 0;

    int p = 0;

    /* Enumerate platforms, we take the first available */
    clgp_clerr =
        clGetPlatformIDs(
                8,
                platforms,
                &n_platforms);
    if (clgp_clerr != CL_SUCCESS) {
        fprintf(stderr, "clgp: Could not check for platforms\n");
        err = CLGP_CL_ERROR;
        goto end;
    }

    *id = NULL;

    /* Enumerate devices */
    for (p = 0; p < n_platforms; p++) {
        clgp_clerr =
            clGetDeviceIDs(
                    platforms[p],
                    CL_DEVICE_TYPE_GPU,
                    16,
                    devices,
                    &n_devs);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "clgp: Could not check for devices\n");
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
    }

end:
    return err;
}

int
clgpFirstCPU(cl_device_id *id)
{
    return clgpFirstDevice(id, CL_DEVICE_TYPE_CPU);
}

