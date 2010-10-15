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

/* Create the pyramid from input as array of (already allocated) climages 
 * whom sizes must be large enough to store the corresponding levels */
int
clgpBuildPyramid(
        cl_mem pyramid_image[],
        cl_mem input_image,
        int width,
        int height);

/* Returns the maximum possible level for an image of size width x height */
int
clgpMaxlevel(int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

