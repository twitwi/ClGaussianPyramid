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

