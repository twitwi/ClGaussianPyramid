#ifndef _CLGP_GAUSS9X9_H_
#define _CLGP_GAUSS9X9_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgpGauss9x9(
        cl_mem output_image, 
        cl_mem input_image,
        int width,
        int height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_GAUSS9X9_H_ */

