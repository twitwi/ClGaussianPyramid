
#ifndef _IPLTORGB_GSP_H_
#define _IPLTORGB_GSP_H_

#include "framework.h"
#include <cxcore.h>

/**
 */
class IPLToRGB {
    
public:
    Framework _framework;
    void input(IplImage *im);
};

CLASS_AS_MODULE(IPLToRGB);

#endif /* ndef _IPLToRGB_GSP_H_ */
