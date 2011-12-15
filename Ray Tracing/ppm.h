#ifndef PPM_H
#define PPM_H

#include <vector>

// A 3-byte structure storing R,G,B value of a pixel
struct PackedPixel {
  unsigned char r,g,b;
};

// The image file is read into `pixels' and its dimension stored into `width'
// and `height'. Throws an exception on error.
void ppmRead(const char *filename, int& width, int& height, std::vector<PackedPixel>& pixels);

// Write image file to a ppm file
void ppmWrite(const char *filename, int width, int height, const std::vector<PackedPixel>& pixels);

#endif
