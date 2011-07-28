#ifndef _LAYOUTUTILS_H_
#define _LAYOUTUTILS_H_

/* Get the x origin of the level in the pyramid image */
static int 
LEVEL_ORIGIN_X(int level, int width, int height) 
{
    return ((level == 0) ? 0 : (int)((( (1.f - powf(0.5f, (float)(level>>1)) ) / (1.f-0.5f)) + 1.f)*(float)width));
}

/* Get the y origin of the level in the pyramid image */
static int 
LEVEL_ORIGIN_Y(int level, int width, int height) 
{
    return ((level <= 2) ? 0 : (level & 0x1)*(height>>(level>>1)));
}

#endif /* ndef _LAYOUTUTILS_H_ */

