#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* Get the x origin of the scale in the pyramid image */
#define SCALE_ORIGIN_X(scale, width, height) \
        (scale == 0) ? 0 : (int)((( (1.f - powf(0.5f, (float)(scale>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width)

/* Get the y origin of the scale in the pyramid image */
#define SCALE_ORIGIN_Y(scale, width, height) \
        (scale <= 2) ? 0 : (scale & 0x1)*(height>>(scale>>1))

/* Initialize the clgp library, must be called before any other function */
int
clgp_init(cl_context context, cl_command_queue queue);

/* Release ressources allocated by the clgp library -- but not the context, 
 * command queue or any images */
void
clgp_release();

/* Returns the maximum possible scale for an image of size width x height */
int
clgp_maxscale(int width, int height);

/* Create the pyramid from input -- both images must be allocated and the 
 * pyramid must be at least of size width*3 x height */
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

