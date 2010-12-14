
#include "PyramidLevelSelector.h"
#include <cv.h>
#include "PyramidLayoutUtils.h"
#include <stdio.h>

PyramidLevelSelector::PyramidLevelSelector() : level(1) {
}

void PyramidLevelSelector::input(IplImage *im) {
    int l = level - 1;
    int w = im->width / 3;
    int h = im->height;
    int x = LEVEL_ORIGIN_X(l, w, h);
    int y = LEVEL_ORIGIN_Y(l, w, h);
    int outW = w >> (l>>1);
    int outH = h >> (l>>1);
    //printf("%d %d %d %d\n", x, y, outW, outH);
    /* // Unfortunately, it does not work as soon as outW != w ... so we copy
    IplImage *out = cvCreateImageHeader(cvSize(outW, outH), IPL_DEPTH_8U, 3);
    out->imageData = im->imageData + x * 3 + y * im->widthStep;
    out->widthStep = im->widthStep;
    emitNamedEvent("output", out);
    cvReleaseImageHeader(&out);
    /*/
    IplImage *out = cvCreateImage(cvSize(outW, outH), IPL_DEPTH_8U, 3);
    cvSetImageROI(im, cvRect(x, y, outW, outH));
    cvCopy(im, out);
    cvResetImageROI(im);
    emitNamedEvent("output", out);
    cvReleaseImage(&out);
    //*/
}
