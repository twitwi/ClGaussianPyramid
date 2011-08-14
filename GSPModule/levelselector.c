#include <assert.h>
#include <stdlib.h>

#ifndef __APPLE__
# include <CL/opencl.h>
#else
# include <OpenCL/opencl.h>
#endif

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct levelselector_module {
    Framework framework;
    size_t level;
};

struct levelselector_module *
LevelSelector__v__create(Framework f)
{
    struct levelselector_module *module = NULL;
    void *levelparam[] = {
        "param",
        "level", "int",
        NULL
    };

    module = malloc(sizeof(struct levelselector_module));
    assert(module != NULL);
    module->framework = f;
    module->level = 0;

    module->framework("param", levelparam);

    return module;
}

/*
void
LevelSelector__v__init(struct levelselector_module *module)
{
}
*/

void
LevelSelector__v__stop(struct levelselector_module *module)
{
    free(module);
}

void
LevelSelector__v__set__v__level(struct levelselector_module *module, int level)
{
    module->level = level;
}

void 
LevelSelector__v__event__v__input(
        struct levelselector_module *module, 
        char **pyramid_bgr,
        int width,
        int height,
        size_t maxlevel)
{
    void *output[] = { 
        "output", 
        "char*", NULL, 
        "int", NULL,
        "int", NULL,
        NULL };
    int level_width = 0;
    int level_height = 0;

    if (module->level >= maxlevel) {
        module->level = maxlevel - 1;
    }

    level_width = width >> ((module->level+1)>>1);
    level_height = height >> (module->level >> 1);

    output[2] = &pyramid_bgr[module->level];
    output[4] = &level_width;
    output[6] = &level_height;
    module->framework("emit", output);
}

void 
LevelSelector__v__event__v__inputClImage(
        struct levelselector_module *module, 
        cl_mem *pyramid_climages,
        size_t maxlevel)
{
    void *output[] = { 
        "output", 
        "PP7_cl_mem", NULL,
        NULL };

    if (module->level >= maxlevel) {
        module->level = maxlevel - 1;
    }

    output[2] = &pyramid_climages[module->level];
    module->framework("emit", output);
}

