
#ifndef _GAUSSPYRAMID_GSP_H_
#define _GAUSSPYRAMID_GSP_H_

#include "framework.h"
#include <CL/cl.h>
#include <cxcore.h>

class GaussPyramid {

    void outputBoth(char* dataRGBA, int wRef, int h);
    struct Cache {
        int w;
        int h;
        char* inRGBA;
        char* outRGBA;
        char* outRGB;
    } cache;
    cl_context context;
    cl_command_queue queue;

public:
    Framework _framework;
    GaussPyramid();

    void initModule();
    void stopModule();
    void input(IplImage* im);
    void inputRGB(char* dataRGB, int w, int h);
    void inputRGBA(char* dataRGBA, int w, int h);
};

CLASS_AS_MODULE(GaussPyramid);
int f() {return 1;}

#endif /* ndef _GAUSSPYRAMID_GSP_H_ */
