#ifndef _CLGP_UTILS_H_
#define _CLGP_UTILS_H_

#ifdef __cplusplus
extern "C" {
#endif

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

/* Retrieve the cl_device_id of the first GPU on the system */
int
clgpFirstGPU(cl_device_id *id);

/* Retrieve the cl_device_id of the more powerful (in terms of flops) GPU on
 * the system */
int
clgpMaxflopsGPU(cl_device_id *id);

/* Retrieve the cl_device_id of the first CPU on the system */
int
clgpFirstCPU(cl_device_id *id);

/* Retrieve the cl_device_id of the first CPU found in vendor's platform,
 * refer to the vendor's documentation to find their vendor strings.
 * Note: platform vendor != device vendor */
int
clgpFirstCPUWithVendor(cl_device_id *id, const char *vendor);

#ifdef __cplusplus
}
#endif

#endif /* ndef _CLGP_UTILS_H_ */

