/* FIXME: For now, the only way I found to pass variables between module
 *        inits is to use a global variable.
 */
#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

cl_command_queue global_command_queue;
