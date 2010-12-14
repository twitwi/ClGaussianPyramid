
#ifndef _PYRAMIDLAYOUTUTILS_GSP_H_
#define _PYRAMIDLAYOUTUTILS_GSP_H_

#include <sys/time.h>

/* Get the x origin of the level in the pyramid image */
static int LEVEL_ORIGIN_X(int level, int width, int height) {
    return ((level == 0) ? 0 : (int)((( (1.f - powf(0.5f, (float)(level>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width));
}
/* Get the y origin of the level in the pyramid image */
static int LEVEL_ORIGIN_Y(int level, int width, int height) {
    return ((level <= 2) ? 0 : (level & 0x1)*(height>>(level>>1)));
}
static double timeFrom(struct timeval start) {
    timeval stop;
    gettimeofday(&stop, NULL);
    double time = (stop.tv_sec-start.tv_sec)*1000.f + (stop.tv_usec-start.tv_usec)/1000.f;
    return time;
}
static double timeFromAndUpdate(struct timeval& start) {
    timeval stop;
    gettimeofday(&stop, NULL);
    double time = (stop.tv_sec-start.tv_sec)*1000.f + (stop.tv_usec-start.tv_usec)/1000.f;
    start = stop;
    return time;
}
#endif /* ndef _PYRAMIDLAYOUTUTILS_GSP_H_ */
