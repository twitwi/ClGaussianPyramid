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

struct ipltobgr_module {
    Framework framework;
};

struct ipltobgr_module *
IplToBGR__v__create(Framework f)
{
    struct ipltobgr_module *module = NULL;

    module = malloc(sizeof(struct ipltobgr_module));
    assert(module != NULL);
    module->framework = f;

    return module;
}

/*
void
IplToBGR__v__init(struct ipltobgr_module *module)
{
}
*/

void
IplToBGR__v__stop(struct ipltobgr_module *module)
{
    free(module);
}

void 
IplToBGR__v__event__v__input(
        struct ipltobgr_module *module, 
        const IplImage *ipl)
{
    void *output[] = { 
        "output", 
        "char*", NULL, 
        "int", NULL, 
        "int", NULL, 
        NULL };
    char *tmp = NULL;
    size_t r = 0;

    /* Fixme: With OpenCV not storing any color format, we're begging for 
     *        color conversion bugs... the best we can do is to check
     *        the number of channel/depth and pray it is BGR (the default 
     *        format used by cvLoadImage and as such, the ImageSource module).
     */
    assert(ipl->nChannels == 3);
    assert(ipl->depth == (int)IPL_DEPTH_8U);

    /* Until we pass a pitch argument, we have to pass non-pitched data */
    if (ipl->widthStep == ipl->width*3*sizeof(char)) {
        tmp = malloc(ipl->width*ipl->height*3*sizeof(char));
        assert(tmp != NULL);
        for (r = 0; r < (size_t)ipl->height; r++) {
            memcpy(tmp + r*ipl->width*3*sizeof(char),
                    ipl->imageData + r*ipl->widthStep,
                    ipl->width*3*sizeof(char));
        }
        output[2] = (void *)&tmp;
    }
    else {
        output[2] = (void *)&ipl->imageData;
    }
    output[4] = (void *)&ipl->width;
    output[6] = (void *)&ipl->height;
    module->framework("emit", output);
    free(tmp);
}

