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
#include <stdio.h>
#include <string.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

static cl_int
clgpFirstDevice(cl_device_id *id, cl_device_type device_type)
{
    cl_int err = 0;

    cl_platform_id platforms[8];
    cl_uint nplatforms = 0;

    cl_device_id devices[16];
    cl_uint ndevices = 0;

    unsigned int p = 0;
    
    *id = NULL;

    /* Enumerate platforms, we take the first available */
    err =
        clGetPlatformIDs(
                8,
                platforms,
                &nplatforms);
    if (err != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: platform get error\n");
#endif
        goto end;
    }

    /* Enumerate devices */
    for (p = 0; p < nplatforms; p++) {
        err =
            clGetDeviceIDs(
                    platforms[p],
                    device_type,
                    16,
                    devices,
                    &ndevices);
        if (err != CL_SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "clgp: device get error\n");
#endif
            goto end;
        }
        if (ndevices > 0) {
            *id = devices[0];
            break;
        }
    }

end:
    return err;
}

cl_int
clgpFirstGPU(cl_device_id *id)
{
    return clgpFirstDevice(id, CL_DEVICE_TYPE_GPU);
}

cl_int
clgpMaxflopsGPU(cl_device_id *id)
{
    cl_int err = 0;

    cl_platform_id platforms[8];
    cl_uint nplatforms = 0;

    cl_device_id devices[16];
    cl_uint ndevices = 0;

    unsigned int maxflops = 0;

    unsigned int d = 0;
    cl_uint d_clock_freq = 0, d_compute_unit_nb = 0;
    unsigned int d_flops = 0;

    unsigned int p = 0;

    /* Enumerate platforms, we take the first available */
    err =
        clGetPlatformIDs(
                8,
                platforms,
                &nplatforms);
    if (err != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: platform get error\n");
#endif
        goto end;
    }

    *id = NULL;

    /* Enumerate devices */
    for (p = 0; p < nplatforms; p++) {
        err =
            clGetDeviceIDs(
                    platforms[p],
                    CL_DEVICE_TYPE_GPU,
                    16,
                    devices,
                    &ndevices);
        if (err != CL_SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "clgp: device get error\n");
#endif
            goto end;
        }

        /* Check each device */
        for (d = 0; d < ndevices; d++) {
            /* Get max clock and compute unit number */
            err = 
                clGetDeviceInfo(
                        devices[d],
                        CL_DEVICE_MAX_CLOCK_FREQUENCY,
                        sizeof(cl_uint),
                        &d_clock_freq,
                        NULL);
            if (err != CL_SUCCESS) {
#ifdef DEBUG
                fprintf(stderr, 
                        "clgp: device info (CL_DEVICE_MAX_CLOCK_FREQUENCY) error\n");
#endif
                goto end;
            }
            err = 
                clGetDeviceInfo(
                        devices[d],
                        CL_DEVICE_MAX_COMPUTE_UNITS,
                        sizeof(cl_uint),
                        &d_compute_unit_nb,
                        NULL);
            if (err != CL_SUCCESS) {
#ifdef DEBUG
                fprintf(stderr, 
                        "clgp: device info (CL_DEVICE_MAX_COMPUTE_UNITS) error\n");
#endif
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

cl_int
clgpFirstCPU(cl_device_id *id)
{
    return clgpFirstDevice(id, CL_DEVICE_TYPE_CPU);
}

cl_int
clgpFirstCPUWithVendor(cl_device_id *id, const char *vendor)
{
    cl_int err = 0;

    cl_platform_id platforms[8];
    cl_uint nplatforms = 0;
    char platform_vendor[1024];

    cl_device_id devices[16];
    cl_uint ndevices = 0;

    size_t vendorlen = 0;

    unsigned int p = 0;
    
    *id = NULL;

    vendorlen = strnlen(vendor, 1024);

    /* Enumerate platforms, we take the first with the right vendor */
    err =
        clGetPlatformIDs(
                8,
                platforms,
                &nplatforms);
    if (err != CL_SUCCESS) {
#ifdef DEBUG
        fprintf(stderr, "clgp: platform get error\n");
#endif
        goto end;
    }

    /* Enumerate devices */
    for (p = 0; p < nplatforms; p++) {
        err =
            clGetPlatformInfo(
                    platforms[p],
                    CL_PLATFORM_VENDOR,
                    1024,
                    platform_vendor,
                    NULL);
        if (err != CL_SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "clgp: platform get vendor error\n");
#endif
            goto end;
        }
        if (strncmp(platform_vendor, vendor, vendorlen) != 0) {
            continue;
        }

        err =
            clGetDeviceIDs(
                    platforms[p],
                    CL_DEVICE_TYPE_CPU,
                    16,
                    devices,
                    &ndevices);
        if (err != CL_SUCCESS) {
#ifdef DEBUG
            fprintf(stderr, "clgp: device get error\n");
#endif
            goto end;
        }
        if (ndevices > 0) {
            *id = devices[0];
            break;
        }
    }

end:
    return err;
}

