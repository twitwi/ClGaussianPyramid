#include <CL/cl.h>

cl_int clgp_clerr;

cl_int
clgp_clerror() {
    return clgp_clerr;
}

