#include <assert.h>

#include <cv.h>

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct rgbtoipl_module {
    Framework framework;
};

struct rgbtoipl_module *
RgbToIpl__v__create(Framework f)
{
    struct rgbtoipl_module *res = NULL;

    res = malloc(sizeof(struct rgbtoipl_module));
    assert(res != NULL);
    res->framework = f;

    return res;
}

/*
void
RgbToIpl__v__init(struct rgbtoipl_module *module)
{
}
*/

void
RgbToIpl__v__stop(struct rgbtoipl_module *module)
{
    free(module);
}

void 
RgbToIpl__v__event__v__input(
        struct rgbtoipl_module *module, 
        unsigned char *data, 
        int width, 
        int height)
{
    void *output[] = { 
        "output", 
        "P9_IplImage", NULL, 
        NULL };
    IplImage *outimage = NULL;

    outimage = cvCreateImageHeader(cvSize(width, height), IPL_DEPTH_8U, 3);
    assert(outimage != NULL);

    outimage->imageData = (char *)data;
    outimage->widthStep = 3 * width;

    output[2] = &outimage;
    module->framework("emit", output);

    cvReleaseImageHeader(&outimage);
}

