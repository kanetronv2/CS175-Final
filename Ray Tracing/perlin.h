/*
    This file belongs to the Ray tracing tutorial of http://www.codermind.com/
    It is free to use for educational purpose and cannot be redistributed
    outside of the tutorial pages.
    Any further inquiry :
    mailto:info@codermind.com
 */

#ifndef __PERLIN_H
#define __PERLIN_H

// Please refer to the website

double noise(double x, double y, double z);

inline float noisef(float x, float y, float z)
{
    return (float) noise((double)x, (double)y, (double)z);
}

#endif // __PERLIN_H
