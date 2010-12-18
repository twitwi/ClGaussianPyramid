#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* Initialize the clgp library, must be called before any other function */
int
clgpInit(cl_context context, cl_command_queue command_queue);

/* Release ressources allocated by the clgp library */
void
clgpRelease(cl_context context, cl_command_queue command_queue);

/* Create the classic half octave pyramid from input and output the 
 * levels in an array of user-allocated climages (whom sizes must be large 
 * enough to store the level). */
int
clgpBuildPyramid(
        cl_command_queue command_queue,
        cl_mem pyramid_image[],
        cl_mem input_image);

/* Returns the maximum possible level for an image of size width x height */
int
clgpMaxlevel(int width, int height);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

