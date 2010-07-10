#ifndef _CLGP_DOWNSCALE_H_
#define _CLGP_DOWNSCALE_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

void
clgp_downscale(
        cl_mem downscaled_image, 
        cl_mem input_image,
        int width,
        int height,
        int scale);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_DOWNSCALE_H */

