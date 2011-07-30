#include <assert.h>
#include <math.h>
#include <string.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include <clgp/clgp.h>
#include <clgp/utils.h>

#include "layoututils.h"

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct gausspyramid_module {
    Framework framework;

    cl_command_queue command_queue;
    cl_kernel *clgpkernels;
};

struct gausspyramid_module *
GaussPyramid__v__create(Framework f)
{
    struct gausspyramid_module *res = NULL;
    
    res = malloc(sizeof(struct gausspyramid_module));
    assert(res != NULL);
    memset(res, 0, sizeof(struct gausspyramid_module));
    res->framework = f;

    return res;
}

void
GaussPyramid__v__init(struct gausspyramid_module *module)
{
    cl_int clerr = 0;
    cl_device_id device = NULL;
    cl_context context = NULL;
    int clgperr = 0;

    clgpMaxflopsGPU(&device);
    assert(device != NULL);

    context = clCreateContext(NULL, 1, &device, NULL, NULL, &clerr);
    assert(clerr == CL_SUCCESS);

    module->command_queue = clCreateCommandQueue(context, device, 0, &clerr);
    assert(clerr == CL_SUCCESS);

    clgperr = clgpInit(context, &module->clgpkernels);
    assert(clgperr == 0);

    clReleaseContext(context);
}

void
GaussPyramid__v__stop(struct gausspyramid_module *module)
{
    clgpRelease(module->clgpkernels);
    clReleaseCommandQueue(module->command_queue);
    free(module);
}

static void
outputboth(
        struct gausspyramid_module *module, 
        const unsigned char *bgra, 
        int width,
        int height)
{
    void *output_bgra[] = {
        "outputBGRA",
        "char*", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    void *output_bgr[] = {
        "outputBGR",
        "char*", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    unsigned char *bgr = NULL;
    size_t x = 0, y  = 0;

    output_bgra[2] = &bgra;
    output_bgra[4] = &width;
    output_bgra[6] = &height;
    module->framework("emit", output_bgra);

    bgr = malloc(width*height*3*sizeof(unsigned char));
    assert(bgr != NULL);
    for(y = 0; y < (size_t)height; y++) {
        for(x = 0; x < (size_t)width; x++) {
            bgr[y*width*3 + x*3 + 0] = bgra[y*width*4 + x*4 + 0];
            bgr[y*width*3 + x*3 + 1] = bgra[y*width*4 + x*4 + 1];
            bgr[y*width*3 + x*3 + 2] = bgra[y*width*4 + x*4 + 2];
        }
    }

    output_bgr[2] = &bgr;
    output_bgr[4] = &width;
    output_bgr[6] = &height;
    module->framework("emit", output_bgr);

    free(bgr);
}

void
GaussPyramid__v__event__v__inputBGRA(
        struct gausspyramid_module *module,
        unsigned char *input_bgra,
        int input_width,
        int input_height)
{
    cl_int clerr = 0;
    int clgperr = 0;

    const cl_image_format clgpimageformat = {CL_BGRA, CL_UNORM_INT8};

    cl_context context = NULL;
    cl_mem input_climage, pyramid_climage[32];
    size_t pyramid_width = input_width*3, pyramid_height = input_height;
    int maxlevel = 0, level = 0;

    unsigned char *pyramid_bgra = NULL;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    clerr =
        clGetKernelInfo(
                module->clgpkernels[0],
                CL_KERNEL_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    input_climage =
        clCreateImage2D(
                context,
                CL_MEM_READ_ONLY,
                &clgpimageformat,
                input_width,
                input_height,
                0,
                NULL,
                &clerr);
    assert(clerr == CL_SUCCESS);

    maxlevel = clgpMaxlevelHalfOctave(input_width, input_height) - 4;
    for (level = 0; level < maxlevel; level++) {
        pyramid_climage[level] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &clgpimageformat,
                    input_width >> (level>>1),
                    input_height >> (level>>1),
                    0,
                    NULL,
                    &clerr);
        assert(clerr == CL_SUCCESS);
    }

    region[0] = input_width;
    region[1] = input_height;
    clerr =
        clEnqueueWriteImage(
                module->command_queue,
                input_climage,
                CL_TRUE,
                origin,
                region,
                0,
                0,
                input_bgra,
                0,
                NULL,
                NULL);
    assert(clerr == CL_SUCCESS);

    clgperr =
        clgpEnqueuePyramidHalfOctave(
            module->command_queue,
            module->clgpkernels,
            pyramid_climage,
            input_climage,
            maxlevel);
    assert(clgperr == 0);

    pyramid_bgra = malloc(pyramid_width*pyramid_height*4*sizeof(unsigned char));
    for (level = 0; level < maxlevel; level++) {
        region[0] = input_width >> (level>>1);
        region[1] = input_height >> (level>>1);
        clerr =
            clEnqueueReadImage(
                    module->command_queue,
                    pyramid_climage[level],
                    CL_TRUE,
                    origin,
                    region,
                    pyramid_width*4*sizeof(unsigned char),
                    0,
                    (void *)((unsigned char *)((char *)pyramid_bgra + LEVEL_ORIGIN_Y(level, input_width, input_height)*pyramid_width*4) + LEVEL_ORIGIN_X(level, input_width, input_height)*4),
                    0,
                    NULL,
                    NULL);
        assert(clerr == CL_SUCCESS);
    }

    clReleaseMemObject(input_climage);
    for (level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climage[level]);
    }

    outputboth(module, pyramid_bgra, pyramid_width, pyramid_height);
    free(pyramid_bgra);
}

void
GaussPyramid__v__event__v__input(
        struct gausspyramid_module *module,
        unsigned char *bgr,
        int width,
        int height)
{
    unsigned char *bgra = NULL;
    size_t x = 0, y = 0;

    bgra = malloc(width*height*4*sizeof(unsigned char));
    assert(bgra != NULL);

    for (y = 0; y < (size_t)height; y++) {
        for (x = 0; x < (size_t)width; x++) {
            bgra[y*width*4 + x*4 + 0] = bgr[y*width*3 + x*3 + 0];
            bgra[y*width*4 + x*4 + 1] = bgr[y*width*3 + x*3 + 1];
            bgra[y*width*4 + x*4 + 2] = bgr[y*width*3 + x*3 + 2];
            bgra[y*width*4 + x*4 + 3] = 255;
        }
    }

    GaussPyramid__v__event__v__inputBGRA(module, bgra, width, height);
    free(bgra);
}

