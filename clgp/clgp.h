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

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_H */

