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

const cl_image_format clgpimageformat = {CL_RGBA, CL_UNSIGNED_INT8};

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
    cl_image_format *imageformats = NULL;
    unsigned int numimageformats = 0;
    unsigned int f = 0;
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
        const unsigned char *rgba, 
        int width,
        int height)
{
    void *output_rgba[] = {
        "outputRGBA",
        "char*", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    void *output_rgb[] = {
        "outputRGB",
        "char*", NULL,
        "int", NULL,
        "int", NULL,
        NULL };
    unsigned char *rgb = NULL;
    size_t x = 0, y  = 0;

    output_rgba[2] = &rgba;
    output_rgba[4] = &width;
    output_rgba[6] = &height;
    module->framework("emit", output_rgba);

    rgb = malloc(width*height*3*sizeof(unsigned char));
    assert(rgb != NULL);
    for(y = 0; y < (size_t)height; y++) {
        for(x = 0; x < (size_t)width; x++) {
            rgb[y*width*3 + x*3 + 0] = rgba[y*width*4 + x*4 + 0];
            rgb[y*width*3 + x*3 + 1] = rgba[y*width*4 + x*4 + 1];
            rgb[y*width*3 + x*3 + 2] = rgba[y*width*4 + x*4 + 2];
        }
    }

    output_rgb[2] = &rgb;
    output_rgb[4] = &width;
    output_rgb[6] = &height;
    module->framework("emit", output_rgb);

    free(rgb);
}

void
GaussPyramid__v__event__v__inputRGBA(
        struct gausspyramid_module *module,
        unsigned char *input_rgba,
        int input_width,
        int input_height)
{
    cl_int clerr = 0;
    int clgperr = 0;

    cl_context context = NULL;
    cl_mem input_climage, pyramid_climage[32];
    size_t pyramid_width = input_width*3, pyramid_height = input_height;
    int maxlevel = 0, level = 0;

    unsigned char *pyramid_rgba = NULL;

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
                input_rgba,
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

    pyramid_rgba = malloc(pyramid_width*pyramid_height*4*sizeof(unsigned char));
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
                    (void *)((unsigned char *)((char *)pyramid_rgba + LEVEL_ORIGIN_Y(level, input_width, input_height)*pyramid_width*4) + LEVEL_ORIGIN_X(level, input_width, input_height)*4),
                    0,
                    NULL,
                    NULL);
        assert(clerr == CL_SUCCESS);
    }

    clReleaseMemObject(input_climage);
    for (level = 0; level < maxlevel; level++) {
        clReleaseMemObject(pyramid_climage[level]);
    }

    outputboth(module, pyramid_rgba, pyramid_width, pyramid_height);
    free(pyramid_rgba);
}

void
GaussPyramid__v__event__v__input(
        struct gausspyramid_module *module,
        unsigned char *rgb,
        int width,
        int height)
{
    unsigned char *rgba = NULL;
    size_t x = 0, y = 0;

    rgba = malloc(width*height*4*sizeof(unsigned char));
    assert(rgba != NULL);

    for (y = 0; y < (size_t)height; y++) {
        for (x = 0; x < (size_t)width; x++) {
            rgba[y*width*4 + x*4 + 0] = rgb[y*width*3 + x*3 + 0];
            rgba[y*width*4 + x*4 + 1] = rgb[y*width*3 + x*3 + 1];
            rgba[y*width*4 + x*4 + 2] = rgb[y*width*3 + x*3 + 2];
            rgba[y*width*4 + x*4 + 3] = 255;
        }
    }

    GaussPyramid__v__event__v__inputRGBA(module, rgba, width, height);
    free(rgba);
}

