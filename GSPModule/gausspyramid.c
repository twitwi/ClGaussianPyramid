#include <assert.h>
#include <math.h>
#include <string.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

#include <clgp/clgp.h>

/*#include "framework.h"*/
typedef void (*Framework) (const char* command, ...);

/* See globalvars.c */
extern cl_command_queue global_command_queue;

struct gausspyramid_module {
    Framework framework;

    cl_command_queue command_queue;
    cl_kernel *clgpkernels;
};

struct gausspyramid_module *
GaussPyramid__v__create(Framework f)
{
    struct gausspyramid_module *module = NULL;
    
    module = malloc(sizeof(struct gausspyramid_module));
    assert(module != NULL);
    memset(module, 0, sizeof(struct gausspyramid_module));
    module->framework = f;

    return module;
}

void
GaussPyramid__v__init(struct gausspyramid_module *module)
{
    cl_int clerr = 0;
    int clgperr = 0;
    cl_context context = NULL;

    module->command_queue = global_command_queue;

    clerr =
        clGetCommandQueueInfo(
                module->command_queue,
                CL_QUEUE_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    clgperr = clgpInit(context, &module->clgpkernels);
    assert(clgperr == 0);
}

void
GaussPyramid__v__stop(struct gausspyramid_module *module)
{
    clgpRelease(module->clgpkernels);
    free(module);
}

static void
outputboth(
        struct gausspyramid_module *module, 
        unsigned char **pyramid_bgra, 
        int width,
        int height,
        int maxlevel)
{
    void *output_bgra[] = {
        "outputRGBA",
        "char*", NULL,
        "int", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    void *output_bgr[] = {
        "outputBGR",
        "char*", NULL,
        "int", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    unsigned char **pyramid_bgr = NULL;
    size_t x = 0, y  = 0;

    output_bgra[2] = &pyramid_bgra;
    output_bgra[4] = &width;
    output_bgra[6] = &height;
    output_bgra[8] = &maxlevel;
    module->framework("emit", output_bgra);

    pyramid_bgr = malloc(maxlevel*sizeof(unsigned char *));
    assert(pyramid_bgr != NULL);
    for (size_t level = 0; level < (size_t)maxlevel; level++) {
        size_t level_width = width >> (level>>1);
        size_t level_height = height >> (level>>1);

        pyramid_bgr[level] = 
            malloc(level_width*level_height*3*sizeof(unsigned char));
        assert(pyramid_bgr != NULL);
        for(y = 0; y < level_height; y++) {
            for(x = 0; x < level_width; x++) {
                pyramid_bgr[level][y*level_width*3 + x*3 + 0] = pyramid_bgra[level][y*level_width*4 + x*4 + 0];
                pyramid_bgr[level][y*level_width*3 + x*3 + 1] = pyramid_bgra[level][y*level_width*4 + x*4 + 1];
                pyramid_bgr[level][y*level_width*3 + x*3 + 2] = pyramid_bgra[level][y*level_width*4 + x*4 + 2];
            }
        }
    }

    output_bgr[2] = &pyramid_bgr;
    output_bgr[4] = &width;
    output_bgr[6] = &height;
    output_bgr[8] = &maxlevel;
    module->framework("emit", output_bgr);

    for (size_t level = 0; level < (size_t)maxlevel; level++) {
        free(pyramid_bgr[level]);
    }
    free(pyramid_bgr);
}

void
GaussPyramid__v__event__v__input(
        struct gausspyramid_module *module,
        cl_mem input_climage)
{
    cl_int clerr = 0;
    int clgperr = 0;

    const cl_image_format clgpimageformat = {CL_RGBA, CL_UNORM_INT8};

    cl_context context = NULL;
    cl_mem pyramid_climage[32];
    size_t width = 0, height = 0, maxlevel = 0, level = 0;

    unsigned char **pyramid_bgra = NULL;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    clerr =
        clGetImageInfo(
                input_climage,
                CL_IMAGE_WIDTH,
                sizeof(size_t),
                &width,
                NULL);
    assert(clerr == CL_SUCCESS);

    clerr =
        clGetImageInfo(
                input_climage,
                CL_IMAGE_HEIGHT,
                sizeof(size_t),
                &height,
                NULL);
    assert(clerr == CL_SUCCESS);

    clerr =
        clGetKernelInfo(
                module->clgpkernels[0],
                CL_KERNEL_CONTEXT,
                sizeof(cl_context),
                &context,
                NULL);
    assert(clerr == CL_SUCCESS);

    maxlevel = clgpMaxlevelHalfOctave(width, height) - 4;
    for (level = 0; level < maxlevel; level++) {
        pyramid_climage[level] =
            clCreateImage2D(
                    context,
                    CL_MEM_READ_WRITE,
                    &clgpimageformat,
                    width >> (level>>1),
                    height >> (level>>1),
                    0,
                    NULL,
                    &clerr);
        assert(clerr == CL_SUCCESS);
    }

    clgperr =
        clgpEnqueuePyramidHalfOctave(
            module->command_queue,
            module->clgpkernels,
            pyramid_climage,
            input_climage,
            maxlevel);
    assert(clgperr == 0);

    pyramid_bgra = malloc(maxlevel*sizeof(unsigned char *));
    assert(pyramid_bgra != NULL);
    for (level = 0; level < maxlevel; level++) {
        region[0] = width >> (level>>1);
        region[1] = height >> (level>>1);

        pyramid_bgra[level] = 
            malloc(region[0]*region[1]*4*sizeof(unsigned char));
        assert(pyramid_bgra[level] != NULL);
        clerr =
            clEnqueueReadImage(
                    module->command_queue,
                    pyramid_climage[level],
                    CL_TRUE,
                    origin,
                    region,
                    0,
                    0,
                    pyramid_bgra[level],
                    0,
                    NULL,
                    NULL);
        assert(clerr == CL_SUCCESS);
        clReleaseMemObject(pyramid_climage[level]);
    }

    outputboth(module, pyramid_bgra, width, height, maxlevel);

    for (level = 0; level < maxlevel; level++) {
        free(pyramid_bgra[level]);
    }
    free(pyramid_bgra);
}

