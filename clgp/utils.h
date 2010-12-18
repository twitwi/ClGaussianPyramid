#ifndef _CLGP_UTILS_H_
#define _CLGP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* Retrieve the cl_device_id of the first GPU on the system */
int
clgpFirstGPU(cl_device_id *id);

/* Retrieve the cl_device_id of the more powerful (in terms of flops) GPU on
 * the system */
int
clgpMaxflopsGPU(cl_device_id *id);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_UTILS_H_ */

