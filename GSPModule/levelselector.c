#include <assert.h>
#include <stdio.h>

#include <cv.h>

#include "layoututils.h"

//#include "framework.h"
typedef void (*Framework) (const char* command, ...);

struct levelselector_module {
    Framework framework;
    int level;
};

struct levelselector_module *
LevelSelector__v__create(Framework f)
{
    struct levelselector_module *res = NULL;
    void *levelparam[] = {
        "param",
        "level", "int",
        NULL
    };

    res = malloc(sizeof(struct levelselector_module));
    assert(res != NULL);
    res->framework = f;
    res->level = 1;

    res->framework("param", levelparam);

    return res;
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
        IplImage *image)
{
    void *output[] = { 
        "output", 
        "P9_IplImage", NULL, 
        NULL };
    size_t level_origin_x = 0, level_origin_y = 0;
    size_t outwidth = (image->width/3) >> (module->level >> 1); 
    size_t outheight = image->height >> (module->level >> 1);
    IplImage *outimage = NULL;

    outimage = 
        cvCreateImage(cvSize(outwidth, outheight), IPL_DEPTH_8U, 3);
    assert(outimage != NULL);

    level_origin_x = 
        LEVEL_ORIGIN_X(module->level, image->width/3, image->height);
    level_origin_y = 
        LEVEL_ORIGIN_Y(module->level, image->width/3, image->height);
    cvSetImageROI(
            image, 
            cvRect(level_origin_x, 
                    level_origin_y, 
                    outimage->width, 
                    outimage->height));
    cvCopy(image, outimage, NULL);
    cvResetImageROI(image);

    output[2] = &outimage;
    module->framework("emit", output);

    cvReleaseImageHeader(&outimage);
}

