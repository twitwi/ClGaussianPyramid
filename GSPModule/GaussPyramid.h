
#ifndef _GAUSSPYRAMID_GSP_H_
#define _GAUSSPYRAMID_GSP_H_

#include "framework.h"

class GaussPyramid {

    void outputBoth(char* dataRGBA, int wRef, int h);

public:
    Framework _framework;

    void initModule();
    void stopModule();
    void input(char* dataRGB, int w, int h);
    void inputRGBA(char* dataRGBA, int w, int h);
};

CLASS_AS_MODULE(GaussPyramid);
int f() {return 1;}

#endif /* ndef _GAUSSPYRAMID_GSP_H_ */
