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
    struct rgbtoipl_module *module = NULL;

    module = malloc(sizeof(struct rgbtoipl_module));
    assert(module != NULL);
    module->framework = f;

    return module;
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
        const unsigned char *rgb, 
        int width, 
        int height)
{
    void *output[] = { 
        "output", 
        "P9_IplImage", NULL, 
        NULL };
    IplImage *image = NULL;
    size_t x = 0, y = 0;

    /* Send an BGR IplImage, it must be what everybody expect */
    image = cvCreateImage(cvSize(width, height), IPL_DEPTH_8U, 3);
    assert(image != NULL);

    for (y = 0; y < (size_t)height; y++) {
        for (x = 0; x < (size_t)width; x++) {
            image->imageData[y*image->widthStep + x*3 + 0] =
                (char)rgb[y*width*3 + x*3 + 2];
            image->imageData[y*image->widthStep + x*3 + 1] =
                (char)rgb[y*width*3 + x*3 + 1];
            image->imageData[y*image->widthStep + x*3 + 2] =
                (char)rgb[y*width*3 + x*3 + 0];
        }
    }

    output[2] = &image;
    module->framework("emit", output);

    cvReleaseImage(&image);
}

