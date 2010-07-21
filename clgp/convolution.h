#ifndef _CLGP_CONVOLUTION_H_
#define _CLGP_CONVOLUTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgp_convolution(
        cl_mem output_image, 
        int output_origin_x,
        int output_origin_y,
        cl_mem input_image,
        int input_origin_x,
        int input_origin_y,
        int width,
        int height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_CONVOLUTION_H_ */

