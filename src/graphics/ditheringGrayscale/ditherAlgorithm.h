/**
 **************************************************
 *
 * @file        ditherAlgorithm.h
 * @brief       Floyd-Steinberg dithering for grayscale EPD panels.
 *              Used by Inkplate boards that operate in 1-bit (black & white)
 *              or 3-bit (8-level grayscale) display modes.
 *              Excluded from builds targeting colour or Inkplate 2 boards,
 *              which use the ditheringColor variant instead.
 *
 * @copyright   GNU Lesser General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#pragma once

#if !defined(ARDUINO_INKPLATECOLOR) && !defined(ARDUINO_INKPLATE2) && !defined(ARDUINO_INKPLATE13SPECTRA)

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <math.h>
#include <float.h>

class Inkplate;

/**
 * @brief   24-bit pixel stored in BGR component order, matching the byte
 *          layout used in BMP files and internally by the dither routines.
 */
typedef struct
{
    uint8_t b, g, r;
} RGBTRIPLE;

/**
 * @brief   Provides Floyd-Steinberg serpentine dithering for grayscale EPD panels.
 *
 *          The algorithm quantises each pixel to the nearest available display
 *          level (1-bit: 0 or 1 — 3-bit: 0–7) and distributes the rounding
 *          error to neighbouring pixels using the standard Floyd-Steinberg
 *          kernel. Rows are processed in alternating left-to-right / right-to-left
 *          order (serpentine scan) to avoid the directional bias that appears
 *          when all rows are scanned in the same direction.
 */
class DitherAlgorithm
{
  public:
    /**
     * @brief   Store the Inkplate instance pointer so the algorithm can write
     *          quantised pixels directly via writePixelInternal().
     *          Must be called once before ditherFramebuffer().
     *
     * @param   inkplatePtr
     *          Pointer to the owning Inkplate instance.
     */
    void begin(Inkplate *inkplatePtr);

    /**
     * @brief   Run Floyd-Steinberg dithering over the entire LVGL framebuffer
     *          and write the quantised pixels to the EPD framebuffer.
     *
     *          The input buffer is the raw L8 (8-bit grayscale) data that LVGL
     *          renders into. Each byte maps to one pixel; values range 0–255.
     *
     * @param   frameBuffer
     *          Pointer to the L8 framebuffer produced by LVGL.
     * @param   width
     *          Framebuffer width in pixels.
     * @param   height
     *          Framebuffer height in pixels.
     * @param   mode
     *          Display mode: 0 = 1-bit (black & white), 1 = 3-bit (8 grayscale levels).
     */
    void ditherFramebuffer(uint8_t *frameBuffer, int width, int height, uint8_t mode);

  private:
    Inkplate *_inkplate;
};

#endif
