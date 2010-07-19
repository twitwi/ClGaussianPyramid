#ifndef _CLGP_OPTIONALLYDOWNSCALEDCONVOLUTION_H_
#define _CLGP_OPTIONALLYDOWNSCALEDCONVOLUTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgp_optionallydownscaledconvolution(
        cl_mem output_image, 
        int out_x_offset,
        int out_y_offset,
        cl_mem input_image,
        int in_x_offset,
        int in_y_offset,
        int in_width,
        int in_height,
        int dowscaleOrNot);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_OPTIONALLYDOWNSCALEDCONVOLUTION_H_ */

