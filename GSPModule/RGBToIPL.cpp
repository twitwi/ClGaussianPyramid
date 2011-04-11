
#include "RGBToIPL.h"

#include <cv.h>

void RGBToIPL::input(char* data, int w, int h) {
    IplImage *out = cvCreateImageHeader(cvSize(w, h), IPL_DEPTH_8U, 3);
    out->imageData = data;
    out->widthStep = 3*w;
    emitNamedEvent("output", out);
    cvReleaseImageHeader(&out);
}

