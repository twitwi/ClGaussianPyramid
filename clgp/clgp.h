#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgp_init(cl_context context, cl_command_queue queue);

void
clgp_release();

int
clgp_maxscale(int width, int height);

int
clgp_pyramid(
        cl_mem *pyramid_images,
        cl_mem input_image,
        int width,
        int height,
        int maxscale);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

