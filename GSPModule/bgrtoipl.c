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

#include <cv.h>

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct bgrtoipl_module {
    Framework framework;
};

struct bgrtoipl_module *
BGRToIpl__v__create(Framework f)
{
    struct bgrtoipl_module *module = NULL;

    module = malloc(sizeof(struct bgrtoipl_module));
    assert(module != NULL);
    module->framework = f;

    return module;
}

/*
void
BGRToIpl__v__init(struct bgrtoipl_module *module)
{
}
*/

void
BGRToIpl__v__stop(struct bgrtoipl_module *module)
{
    free(module);
}

void 
BGRToIpl__v__event__v__input(
        struct bgrtoipl_module *module, 
        const char *bgr, 
        int width, 
        int height)
{
    void *output[] = { 
        "output", 
        "P9_IplImage", NULL, 
        NULL };
    IplImage *ipl = NULL;

    /* Send an BGR IplImage, it must be what everybody expect */
    ipl = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 3);
    assert(ipl != NULL);

    ipl->imageData = (char *)bgr;
    ipl->widthStep = width*3*sizeof(char);

    output[2] = &ipl;
    module->framework("emit", output);

    cvReleaseImageHeader(&ipl);
}

