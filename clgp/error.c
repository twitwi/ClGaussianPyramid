#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

cl_int clgp_clerr;

cl_int
clgpClError() {
    return clgp_clerr;
}

