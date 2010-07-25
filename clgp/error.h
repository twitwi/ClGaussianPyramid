#ifndef _CLGP_ERROR_H_
#define _CLGP_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

/* clgp errors, returned by all clgp_ functions */
#define CLGP_CL_ERROR -1

/* Retrieve the status of the last OpenCL operation */
cl_int
clgpClError();

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_ERROR_H_ */

