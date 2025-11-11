#pragma once

#ifdef USE_COLOR_IMAGE

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

class Inkplate;

// 24-bit pixel (BGR order in BMP)
typedef struct {
    uint8_t b, g, r;
} RGBTRIPLE;

class DitherAlgorithm{
    public:
        void ditherFramebuffer(uint8_t* frameBuffer, int width, int height);
        void begin(uint16_t* palette, uint8_t* paletteIndices, uint8_t paletteSize, Inkplate* inkplatePtr);
    private:
        uint8_t color_index;
        Inkplate* _inkplate;
        uint8_t palette_size;
        uint16_t* _palette;
        uint8_t* _paletteIndices;

        RGBTRIPLE map_pixel_classic(int _r, int _g, int _b, uint16_t* palette, uint8_t* palette_indices, uint8_t palette_size);
        RGBTRIPLE map_pixel_fast(int _r, int _g, int _b);
        void RGB_to_HSV(int _r, int _g, int _b, float *h, float *s, float *v);
        float HSV_distance(float h1, float s1, float v1, float h2, float s2, float v2);
        RGBTRIPLE map_pixel_HSV(int _r, int _g, int _b, uint16_t* palette, uint8_t* palette_indices, uint8_t palette_size);
        uint8_t clampValue(int32_t _value, int32_t _min, int32_t _max);
        void RGB565_to_RGBtriple(uint16_t c, uint8_t *r, uint8_t *g, uint8_t *b);


};

#endif