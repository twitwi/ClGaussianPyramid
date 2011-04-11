#ifndef _CLGP_DOWNSAMPLEDGAUSS5X5_H_
#define _CLGP_DOWNSAMPLEDGAUSS5X5_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgpDownsampledGauss5x5_cols(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem output_image, 
        cl_mem input_image,
        size_t width,
        size_t height);

int
clgpDownsampledGauss5x5_rows(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem output_image, 
        cl_mem input_image,
        size_t width,
        size_t height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_DOWNSAMPLEDGAUSS5X5_H_ */

