
#include "IPLToRGB.h"

#include <cv.h>
#include <stdio.h>

void IPLToRGB::input(IplImage *im) {
    char *toFree = NULL;
    char *data;
    int w = im->width;
    int h = im->height;
    if (im->widthStep == w*3) {
        data = im->imageData;
    } else {
        printf("not sure it works\n");
        toFree = data = (char*)malloc(w*h*3);
        IplImage *tmp = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 3);
        tmp->imageData = data;
        tmp->widthStep = w*3;
        cvGetSubRect(im, (CvMat*)tmp, cvRect(0,0,w,h));
        cvReleaseImageHeader(&tmp);
    }
    emitNamedEvent("output", data, w, h);
    if (toFree) free(toFree);
}

