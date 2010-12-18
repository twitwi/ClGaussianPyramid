#ifndef _CLGP_DOWNSCALEDGAUSS5X5_H_
#define _CLGP_DOWNSCALEDGAUSS5X5_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgp_downscaledgauss5x5_program(
        cl_command_queue command_queue,
        cl_mem output_image, 
        cl_mem input_image,
        size_t width,
        size_t height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_DOWNSCALEDGAUSS5X5_H_ */

