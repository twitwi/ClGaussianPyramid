#ifndef _CLGP_ERROR_H_
#define _CLGP_ERROR_H_

#ifdef __cplusplus
extern "C" {
#endif

#include <CL/cl.h>

#define CLGP_CL_ERROR -1

cl_int
clgp_clerror();

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_ERROR_H_ */

