#ifndef _CLGP_UTILS_H_
#define _CLGP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* Retrieve the cl_device_id of the first GPU on the system */
int
clgpFirstDevice(cl_device_id *id);

/* Retrieve the cl_device_id of the more powerful (in terms of flops) GPU on
 * the system */
int
clgpMaxflopsDevice(cl_device_id *id);

/* Create a 2D image object, simplified version (for GPUs only) */
cl_mem
clgpCreateImage2D (
        cl_mem_flags flags, 
        const cl_image_format *image_format, 
        size_t image_width, 
        size_t image_height, 
        cl_int *errcode_ret);

/* Create a 2D image object that will store the pyramid (ie allocate
 * an image of size width*3 x height) */
cl_mem
clgpCreatePyramid2D(
        cl_mem image,
        cl_int *errcode_ret);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_UTILS_H_ */

