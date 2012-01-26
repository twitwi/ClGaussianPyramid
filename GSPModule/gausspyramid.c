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
#include <assert.h>
#include <math.h>
#include <string.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include <clgp/clgp.h>

/*#include "framework.h"*/
typedef void (*Framework) (const char* command, ...);

/* See globalvars.c */
extern cl_command_queue global_command_queue;

struct gausspyramid_module {
    Framework framework;

    cl_command_queue command_queue;
    cl_kernel *clgpkernels;
};

struct gausspyramid_module *
GaussPyramid__v__create(Framework f)
{
    struct gausspyramid_module *module = NULL;
    
    module = malloc(sizeof(struct gausspyramid_module));
    assert(module != NULL);
    memset(module, 0, sizeof(struct gausspyramid_module));
    module->framework = f;

    return module;
}

void
GaussPyramid__v__init(struct gausspyramid_module *module)
{
    cl_int clerr = 0;
    int clgperr = 0;
    cl_context context = NULL;

    module->command_queue = global_command_queue;

    clerr =
        clGetCommandQueueInfo(
                module->command_queue,
                CL_QUEUE_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    clgperr = clgpInit(context, &module->clgpkernels);
    assert(clgperr == 0);
}

void
GaussPyramid__v__stop(struct gausspyramid_module *module)
{
    clgpRelease(module->clgpkernels);
    free(module);
}

void
GaussPyramid__v__event__v__input(
        struct gausspyramid_module *module,
        cl_mem input_climage)
{
    void *output[] = {
        "output",
        "PP7_cl_mem", NULL,
        "int", NULL,
        NULL };

    cl_int clerr = 0;
    int clgperr = 0;

    cl_context context = NULL;
    cl_image_format imageformat;
    cl_mem *pyramid_climages;
    size_t width = 0, height = 0, maxlevel = 0;

    clerr =
        clGetImageInfo(
                input_climage,
                CL_IMAGE_FORMAT,
                sizeof(cl_image_format),
                &imageformat,
                NULL);
    assert(clerr == CL_SUCCESS);

    clerr =
        clGetImageInfo(
                input_climage,
                CL_IMAGE_WIDTH,
                sizeof(size_t),
                &width,
                NULL);
    assert(clerr == CL_SUCCESS);

    clerr =
        clGetImageInfo(
                input_climage,
                CL_IMAGE_HEIGHT,
                sizeof(size_t),
                &height,
                NULL);
    assert(clerr == CL_SUCCESS);

    clerr =
        clGetKernelInfo(
                module->clgpkernels[0],
                CL_KERNEL_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    maxlevel = clgpMaxlevelHalfOctave(width, height) - 7;
    pyramid_climages = malloc(maxlevel*sizeof(cl_mem));
    for (size_t level = 0; level < maxlevel; level++) {
        pyramid_climages[level] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &imageformat,
                    width >> ((level+1)>>1),
                    height >> (level>>1),
                    0,
                    NULL,
                    &clerr);
        assert(clerr == CL_SUCCESS);
    }

    clgperr =
        clgpEnqueuePyramidSqrt2(
            module->command_queue,
            module->clgpkernels,
            pyramid_climages,
            input_climage,
            maxlevel);
    assert(clgperr == 0);

    output[2] = &pyramid_climages;
    int maxlevel_as_int = maxlevel; /* Why no size_t in GSP?? */
    output[4] = &maxlevel_as_int;
    module->framework("emit", output);

    for (size_t level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climages[level]);
    }
    free(pyramid_climages);
}

