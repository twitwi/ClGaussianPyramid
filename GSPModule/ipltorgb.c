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
    struct ipltorgb_module *res = NULL;

    res = malloc(sizeof(struct ipltorgb_module));
    assert(res != NULL);
    res->framework = f;

    return res;
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
        IplImage *image)
{
    void *output[] = { 
        "output", 
        "unsigned char *", NULL, 
        "int", NULL, 
        "int", NULL, 
        NULL };
    unsigned char *outdata = NULL, *tmpdata = NULL;
    int r = 0;

    /* Fixme: With OpenCV not storing any color format, we're begging for 
     *        color conversion bugs... the best we can do is to check
     *        the number of channel/depth and pray it is RGB (and not BGR for
     *        example...)
     */
    assert(image->nChannels == 3);
    assert(image->depth == (int)IPL_DEPTH_8U || image->depth == (int)IPL_DEPTH_8S);

    if (image->widthStep == image->width * 3) {
        outdata = (unsigned char *)image->imageData;
    }
    else {
        outdata = tmpdata = 
            malloc(image->width * image->height * 3 * sizeof(unsigned char));
        for (r = 0; r < image->height; r++) {
            memcpy(outdata + r*image->width*3,
                    image->imageData + r*image->widthStep,
                    image->width*3*sizeof(unsigned char));
        }
    }

    output[2] = &outdata;
    output[4] = &image->width;
    output[6] = &image->height;
    module->framework("emit", output);

    free(tmpdata);
}

