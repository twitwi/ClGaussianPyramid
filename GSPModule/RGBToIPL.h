
#ifndef _RGBTOIPL_GSP_H_
#define _RGBTOIPL_GSP_H_

#include "framework.h"
#include <cxcore.h>

/**
 */
class RGBToIPL {
    
public:
    Framework _framework;
    void input(char* data, int w, int h);
};

CLASS_AS_MODULE(RGBToIPL);

#endif /* ndef _RGBTOIPL_GSP_H_ */
