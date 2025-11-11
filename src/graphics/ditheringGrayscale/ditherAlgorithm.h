#pragma once

#if !defined(ARDUINO_INKPLATECOLOR) && !defined(ARDUINO_INKPLATE2)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

class Inkplate;

// 24-bit pixel (BGR order in BMP)
typedef struct
{
    uint8_t b, g, r;
} RGBTRIPLE;

class DitherAlgorithm
{
  public:
    void ditherFramebuffer(uint8_t *frameBuffer, int width, int height, uint8_t mode);
    void begin(Inkplate *inkplatePtr);

  private:
    Inkplate *_inkplate;
};

#endif