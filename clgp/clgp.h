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
#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

/* Initialize the clgp library, must be called before any other function */
int
clgpInit(cl_context context, cl_kernel **kernelsptr);

/* Release ressources allocated by the clgp library */
void
clgpRelease(cl_kernel *kernels);

/* Returns the maximum possible level for an image of size width x height in 
 * the classic pyramid */
size_t
clgpMaxlevel(size_t width, size_t height);

/* Create the classic gaussian pyramid from input and store its levels in
 * the pyramid_image array images (whom sizes must be large enough to store 
 * the corresponding level). */
int
clgpEnqueuePyramid(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        size_t maxlevel);

/* Returns the maximum possible level for an image of size width x height in 
 * the half-octave pyramid (ie the double of the normal pyramid) */
size_t
clgpMaxlevelHalfOctave(size_t width, size_t height);

/* Create the half-octave gaussian pyramid from input and store its levels in
 * the pyramid_image array images (whom sizes must be large enough to store 
 * the corresponding level). */
int
clgpEnqueuePyramidHalfOctave(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        size_t maxlevel);

/* Create the half-octave gaussian pyramid with its true layout from input and 
 * store its levels in the pyramid_image array images (whom sizes must be large 
 * enough to store the corresponding level). */
int
clgpEnqueuePyramidSqrt2(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        size_t maxlevel);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

