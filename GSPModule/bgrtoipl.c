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

