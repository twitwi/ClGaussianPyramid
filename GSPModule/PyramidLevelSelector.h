
#ifndef _PYRAMIDLEVELSELECTOR_GSP_H_
#define _PYRAMIDLEVELSELECTOR_GSP_H_

#include "framework.h"
#include <cxcore.h>

/**
 * This module just extracts a subpart of the in-single-image pyramid.
 */
class PyramidLevelSelector {
    
public:
    Framework _framework;
    FIELD_WITH_SETTER(int,level,setLevel); // defaults to the min of 1 (0 would be the raw image)
    PyramidLevelSelector();

    void input(IplImage* im);
};

CLASS_AS_MODULE(PyramidLevelSelector);

#endif /* ndef _PYRAMIDLEVELSELECTOR_GSP_H_ */
