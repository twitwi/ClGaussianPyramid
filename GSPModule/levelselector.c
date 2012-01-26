/* Copyright (c) 2009-2012 Matthieu Volat and Remi Emonet. All rights
 * reserved.
 * 
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
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

