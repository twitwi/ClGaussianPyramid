#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

#define SCALE_ORIGIN_X(scale, width, height) \
    ((scale != 0)*width)

#define SCALE_ORIGIN_Y(scale, width, height) \
    ((scale >= 2) ? (int)((( (1.f - powf(0.5f, (float)scale)) / (1.f-0.5f) ) - 1.f)*(float)height) : 0) 

int
clgp_init(cl_context context, cl_command_queue queue);

void
clgp_release();

int
clgp_maxscale(int width, int height);

int
clgp_pyramid(
        cl_mem pyramid_image,
        cl_mem input_image,
        int width,
        int height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

