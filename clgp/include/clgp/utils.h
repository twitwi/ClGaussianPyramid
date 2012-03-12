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
#ifndef _CLGP_UTILS_H_
#define _CLGP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

/* Retrieve the cl_device_id of the first GPU on the system */
int
clgpFirstGPU(cl_device_id *id);

/* Retrieve the cl_device_id of the more powerful (in terms of flops) GPU on
 * the system */
int
clgpMaxflopsGPU(cl_device_id *id);

/* Retrieve the cl_device_id of the first CPU on the system */
int
clgpFirstCPU(cl_device_id *id);

/* Retrieve the cl_device_id of the first CPU found in vendor's platform,
 * refer to the vendor's documentation to find their vendor strings.
 * Note: platform vendor != device vendor */
int
clgpFirstCPUWithVendor(cl_device_id *id, const char *vendor);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_UTILS_H_ */

