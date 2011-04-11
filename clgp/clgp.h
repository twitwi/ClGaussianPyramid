#ifndef _CLGP_H_
#define _CLGP_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* Initialize the clgp library, must be called before any other function */
int
clgpInit(cl_context context, cl_kernel **kernels);

/* Release ressources allocated by the clgp library */
void
clgpRelease(cl_context context, cl_kernel *kernels);

/* Returns the maximum possible level for an image of size width x height in 
 * the half-octave pyramid (ie the double of the normal pyramid) */
int
clgpMaxlevelHalfOctave(size_t width, size_t height);

/* Create the half-octave gaussian pyramid from input and store its levels in
 * the pyramid_image array images (whom sizes must be large enough to store 
 * the corresponding level). */
int
clgpBuildPyramidHalfOctave(
        cl_command_queue command_queue,
        cl_kernel *kernels,
        cl_mem pyramid_image[],
        cl_mem input_image,
        int maxlevel);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H_ */

