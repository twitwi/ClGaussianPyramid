#ifndef _CLGP_UTILS_H_
#define _CLGP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

int
clgp_first_device(cl_device_id *id);

int
clgp_maxflops_device(cl_device_id *id);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_UTILS_H_ */

