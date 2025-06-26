#ifndef WRITEPNG_H
#define WRITEPNG_H

#include <stdio.h>
#include <stdlib.h>
#include <png.h>

// Function to write a PNG image
void writePNGImage(
    int* data,
    int width, int height,
    const char* filename,
    int maxIterations);

#endif // WRITEPNG_H