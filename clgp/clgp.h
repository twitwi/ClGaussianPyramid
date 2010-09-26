#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* Initialize the clgp library, must be called before any other function */
int
clgpInit(cl_context context, cl_command_queue queue);

/* Release ressources allocated by the clgp library */
void
clgpRelease();

/* Create the pyramid from input -- both images must be allocated and the 
 * pyramid must be at least of size width*3 x height */
int
clgpBuildPyramid(
        cl_mem pyramid_image,
        cl_mem input_image,
        int width,
        int height);

/* Returns the maximum possible level for an image of size width x height */
int
clgpMaxlevel(int width, int height);

/* Get the x origin of the level in the pyramid image */
#define LEVEL_ORIGIN_X(level, width, height) \
        (level == 0) \
			? 0 \
			: (int)((( (1.f - powf(0.5f, (float)(level>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width)

/* Get the y origin of the level in the pyramid image */
#define LEVEL_ORIGIN_Y(level, width, height) \
        (level <= 2) ? 0 : (level & 0x1)*(height>>(level>>1))

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

