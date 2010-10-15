#ifndef _CLGP_DOWNLEVELDCONVOLUTION_H_
#define _CLGP_DOWNLEVELDCONVOLUTION_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgpDownscaledConvolution(
        cl_mem output_image, 
        cl_mem input_image,
        int width,
        int height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_DOWNLEVELDCONVOLUTION_H_ */

