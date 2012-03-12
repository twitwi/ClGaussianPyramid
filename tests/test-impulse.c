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

#include <clgp.h>
#include <utils.h>

int
main(void)
{
    cl_int err = 0;

    cl_device_id device = NULL;
    cl_context context = NULL;
    cl_command_queue queue = NULL;
    cl_kernel *clgpkernels = NULL;

    unsigned char *input_data = NULL, *pyramid_data[32];
    size_t input_width = 0;
    size_t input_height = 0;
    size_t input_nbchannels = 0;

    cl_image_format imageformat = {CL_RGBA, CL_UNORM_INT8};
    cl_mem input_climage, pyramid_climage[32];
    size_t maxlevel = 0, level = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    clgpFirstGPU(&device);
    if (device == NULL) {
        fprintf(stderr, "No device available\n");
        exit(1);
    }

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not create a context for the device\n");
        exit(1);
    }

    queue = clCreateCommandQueue(context, device, 0, &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not create a command queue for the device\n");
        exit(1);
    }

    if (clgpInit(context, &clgpkernels) != 0) {
        fprintf(stderr, "Could not init clgp library\n");
        exit(1);
    }

    input_width = 65;
    input_height = 65;
    input_nbchannels = 4;
    input_data = malloc(input_width*input_height*input_nbchannels);
    if (input_data == NULL) {
        fprintf(stderr, "Could not allocate input buffer on host\n");
        exit(1);
    }
    memset(input_data, 0, input_width*input_height*input_nbchannels);
    for (size_t c = 0; c < input_nbchannels; c++) {
        input_data[32*input_width*input_nbchannels + 32*input_nbchannels + c] = 
            255;
    }
    input_climage = 
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
                &imageformat,
                input_width,
                input_height,
                0,
                input_data,
                &err);
    if (err != CL_SUCCESS) {
        fprintf(stderr, "Could not create input_climage\n");
        exit(1);
    }

    maxlevel = clgpMaxlevelHalfOctave(input_width, input_height) - 8;
    for (level = 0; level < maxlevel; level++) {
        size_t level_width = input_width >> ((level+1)>>1);
        size_t level_height = input_height >> (level>>1);
        pyramid_data[level] = 
            malloc(level_width*level_height*input_nbchannels);
        if (pyramid_data[level] == NULL) {
            fprintf(stderr, 
                    "Could not allocate pyramid buffer %lu on host\n",
                    level);
            exit(1);
        }
        pyramid_climage[level] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &imageformat,
                    level_width,
                    level_height,
                    0,
                    NULL,
                    &err);
        if (err != CL_SUCCESS) {
            fprintf(stderr, "Could not allocate pyramid_climage[%lu]\n", level);
            exit(1);
        }
    }

    err = 
        clgpEnqueuePyramidSqrt2(
                queue,
                clgpkernels,
                pyramid_climage, 
                input_climage,
                maxlevel);
    if (err != 0) {
        fprintf(stderr, "Pyramid failed\n");
        exit(1);
    }

    for (level = 0; level < maxlevel; level++) {
        region[0] = input_width >> ((level+1)>>1);
        region[1] = input_height >> (level>>1);
        err =
            clEnqueueReadImage(
                    queue,
                    pyramid_climage[level],
                    CL_TRUE,
                    origin,
                    region,
                    0,
                    0,
                    pyramid_data[level],
                    0,
                    NULL,
                    NULL);
        if (err != CL_SUCCESS) {
            fprintf(stderr,
                    "Could not copy pyramid data on host (%d)\n", err);
            exit(1);
        }
        printf(" * level %lu :\n", level);
        for (size_t x = 0; x < region[0]; x++) {
            printf("%d,", pyramid_data[level][region[1]/2*region[0]*4 + x*4]);
        }
        printf("\n");
    }

    clReleaseMemObject(input_climage);
    free(input_data);
    for (level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climage[level]);
        free(pyramid_data[level]);
    }

    clgpRelease(clgpkernels);
    clReleaseCommandQueue(queue);
    clReleaseContext(context);

    return 0;
}

