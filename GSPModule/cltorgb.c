#include <assert.h>
#include <stdlib.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

/* See globalvars.c */
extern cl_command_queue global_command_queue;

struct cltorgb_module {
    Framework framework;

    cl_command_queue command_queue;
};

struct cltorgb_module *
ClToRGB__v__create(Framework f)
{
    struct cltorgb_module *module = NULL;

    module = malloc(sizeof(struct cltorgb_module));
    assert(module != NULL);
    module->framework = f;

    return module;
}

void
ClToRGB__v__init(struct cltorgb_module *module)
{
    module->command_queue = global_command_queue;
}

void
ClToRGB__v__stop(struct cltorgb_module *module)
{
    free(module);
}

void 
ClToRGB__v__event__v__input(
        struct cltorgb_module *module, 
        cl_mem climage)
{
    void *output[] = { 
        "output", 
        "char*", NULL, 
        "int", NULL, 
        "int", NULL, 
        NULL };
    size_t width = 0, height = 0;
    unsigned char *rgba = NULL, *rgb = NULL;

    cl_int clerr = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    clerr =
        clGetImageInfo(
                climage,
                CL_IMAGE_WIDTH,
                sizeof(size_t),
                &width,
                NULL);
    assert(clerr == CL_SUCCESS);
    clerr =
        clGetImageInfo(
                climage,
                CL_IMAGE_HEIGHT,
                sizeof(size_t),
                &height,
                NULL);
    assert(clerr == CL_SUCCESS);

    region[0] = width;
    region[1] = height;

    rgba = malloc(region[0]*region[1]*4*sizeof(unsigned char));
    assert(rgba != NULL);
    clerr =
        clEnqueueReadImage(
                module->command_queue,
                climage,
                CL_TRUE,
                origin,
                region,
                0,
                0,
                rgba,
                0,
                NULL,
                NULL);
    assert(clerr == CL_SUCCESS);

    rgb = malloc(region[0]*region[1]*3*sizeof(unsigned char));
    assert(rgb != NULL);
    for (size_t y = 0; y < region[1]; y++) {
        for (size_t x = 0; x < region[0]; x++) {
            rgb[y*region[0]*3 + x*3 + 0] = rgba[y*region[0]*4 + x*4 + 0];
            rgb[y*region[0]*3 + x*3 + 1] = rgba[y*region[0]*4 + x*4 + 1];
            rgb[y*region[0]*3 + x*3 + 2] = rgba[y*region[0]*4 + x*4 + 2];
        }
    }

    output[2] = (void *)&rgb;
    int width_as_int = width; /* Why no size_t in GSP?? */
    output[4] = (void *)&width_as_int;
    int height_as_int = height; /* Why no size_t in GSP?? */
    output[6] = (void *)&height_as_int;
    module->framework("emit", output);

    free(rgba);
    free(rgb);
}

void 
ClToRGB__v__event__v__inputPyramid(
        struct cltorgb_module *module, 
        cl_mem *pyramid_climages,
        int maxlevel)
{
    void *output[] = { 
        "output", 
        "char*", NULL, 
        "int", NULL, 
        "int", NULL, 
        "int", NULL,
        NULL };
    size_t width = 0, height = 0;
    unsigned char **rgba = NULL, **rgb = NULL;

    cl_int clerr = 0;

    size_t origin[3] = {0, 0, 0};
    size_t region[3] = {0, 0, 1};

    clerr =
        clGetImageInfo(
                pyramid_climages[0],
                CL_IMAGE_WIDTH,
                sizeof(size_t),
                &width,
                NULL);
    assert(clerr == CL_SUCCESS);
    clerr =
        clGetImageInfo(
                pyramid_climages[0],
                CL_IMAGE_HEIGHT,
                sizeof(size_t),
                &height,
                NULL);
    assert(clerr == CL_SUCCESS);

    rgba = malloc(maxlevel*sizeof(unsigned char *));
    assert(rgba != NULL);
    rgb = malloc(maxlevel*sizeof(unsigned char *));
    assert(rgb != NULL);
    for (size_t level = 0; level < (size_t)maxlevel; level++) {
        region[0] = width >> (level>>1);
        region[1] = height >> (level>>1);

        rgba[level] = 
            malloc(region[0]*region[1]*4*sizeof(unsigned char));
        assert(rgba != NULL);
        clerr =
            clEnqueueReadImage(
                    module->command_queue,
                    pyramid_climages[level],
                    CL_TRUE,
                    origin,
                    region,
                    0,
                    0,
                    rgba[level],
                    0,
                    NULL,
                    NULL);
        assert(clerr == CL_SUCCESS);

        rgb[level] = 
            malloc(region[0]*region[1]*3*sizeof(unsigned char));
        assert(rgb[level] != NULL);
        for (size_t y = 0; y < region[1]; y++) {
            for (size_t x = 0; x < region[0]; x++) {
                rgb[level][y*region[0]*3 + x*3 + 0] = rgba[level][y*region[0]*4 + x*4 + 0];
                rgb[level][y*region[0]*3 + x*3 + 1] = rgba[level][y*region[0]*4 + x*4 + 1];
                rgb[level][y*region[0]*3 + x*3 + 2] = rgba[level][y*region[0]*4 + x*4 + 2];
            }
        }
    }

    output[2] = (void *)&rgb;
    int width_as_int = width; /* Why no size_t in GSP?? */
    output[4] = (void *)&width_as_int;
    int height_as_int = height; /* Why no size_t in GSP?? */
    output[6] = (void *)&height_as_int;
    output[8] = &maxlevel;
    module->framework("emit", output);

    for (size_t level = 0; level < (size_t)maxlevel; level++) {
        free(rgba[level]);
        free(rgb[level]);
    }
    free(rgba);
    free(rgb);
}

