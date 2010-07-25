#include <CL/cl.h>

cl_int clgp_clerr;

cl_int
clgpClError() {
    return clgp_clerr;
}

