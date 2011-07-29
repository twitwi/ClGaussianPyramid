#include <assert.h>

#include <cv.h>

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct ipltorgb_module {
    Framework framework;
};

struct ipltorgb_module *
IplToRgb__v__create(Framework f)
{
    struct ipltorgb_module *module = NULL;

    module = malloc(sizeof(struct ipltorgb_module));
    assert(module != NULL);
    module->framework = f;

    return module;
}

/*
void
IplToRgb__v__init(struct ipltorgb_module *module)
{
}
*/

void
IplToRgb__v__stop(struct ipltorgb_module *module)
{
    free(module);
}

void 
IplToRgb__v__event__v__input(
        struct ipltorgb_module *module, 
        const IplImage *image)
{
    void *output[] = { 
        "output", 
        "char*", NULL, 
        "int", NULL, 
        "int", NULL, 
        NULL };
    unsigned char *rgb = NULL;
    size_t x = 0, y = 0;

    /* Fixme: With OpenCV not storing any color format, we're begging for 
     *        color conversion bugs... the best we can do is to check
     *        the number of channel/depth and pray it is BGR (the default 
     *        format used by cvLoadImage and as such, the ImageSource module).
     */
    assert(image->nChannels == 3);
    assert(image->depth == (int)IPL_DEPTH_8U 
            || image->depth == (int)IPL_DEPTH_8S);

    rgb = malloc(image->width * image->height * 3 * sizeof(unsigned char));
    assert(rgb != NULL);

    for (y = 0; y < (size_t)image->height; y++) {
        for (x = 0; x < (size_t)image->width; x++) {
            rgb[y*image->width*3 + x*3 + 0] = 
                (unsigned char)image->imageData[y*image->widthStep + x*3 + 2];
            rgb[y*image->width*3 + x*3 + 1] = 
                (unsigned char)image->imageData[y*image->widthStep + x*3 + 1];
            rgb[y*image->width*3 + x*3 + 2] = 
                (unsigned char)image->imageData[y*image->widthStep + x*3 + 0];
        }
    }

    output[2] = &rgb;
    output[4] = (void *)&image->width;
    output[6] = (void *)&image->height;
    module->framework("emit", output);

    free(rgb);
}

