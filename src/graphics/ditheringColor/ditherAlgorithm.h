/**
 **************************************************
 *
 * @file        ditherAlgorithm.h
 * @brief       Floyd-Steinberg dithering for colour EPD panels.
 *              Used by Inkplate boards that use a fixed colour palette
 *              (Inkplate 6COLOR, Inkplate 2, Inkplate 13 Spectra).
 *              Excluded from grayscale board builds, which use the
 *              ditheringGrayscale variant instead.
 *
 *              Three palette-matching strategies are implemented:
 *                - Classic  — luminance-weighted Euclidean distance in RGB.
 *                - Fast     — unweighted Euclidean distance with luma/saturation
 *                             tie-breaking (used by default in ditherFramebuffer).
 *                - HSV      — nearest-neighbour match in HSV colour space.
 *
 * @copyright   GNU Lesser General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#pragma once

#if defined(ARDUINO_INKPLATECOLOR) || defined(ARDUINO_INKPLATE2) || defined(ARDUINO_INKPLATE13SPECTRA)

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
 *          All colour components are stored at their native RGB565 precision
 *          (R: 5 bits, G: 6 bits, B: 5 bits).
 */
typedef struct
{
    uint8_t b, g, r;
} RGBTRIPLE;

/**
 * @brief   Provides Floyd-Steinberg serpentine dithering for colour EPD panels
 *          that render from a small fixed palette (e.g. black, white, red,
 *          green, blue, yellow, orange).
 *
 *          The LVGL framebuffer is RGB565. Each pixel is mapped to the closest
 *          palette entry, the quantisation error is distributed to unprocessed
 *          neighbours using the Floyd-Steinberg kernel, and the chosen palette
 *          index is written to the EPD framebuffer via writePixelInternal().
 *          Rows alternate scan direction (serpentine) to reduce banding.
 */
class DitherAlgorithm
{
  public:
    /**
     * @brief   Initialise the dithering engine with the board's colour palette.
     *          Copies the palette and index arrays into PSRAM and stores the
     *          Inkplate pointer for pixel writes. Must be called once before
     *          ditherFramebuffer().
     *
     * @param   palette
     *          Array of RGB565 colour values representing the panel's palette.
     * @param   paletteIndices
     *          Array of EPD colour indices corresponding to each palette entry.
     * @param   paletteSize
     *          Number of entries in the palette.
     * @param   inkplatePtr
     *          Pointer to the owning Inkplate instance.
     */
    void begin(uint16_t *palette, uint8_t *paletteIndices, uint8_t paletteSize, Inkplate *inkplatePtr);

    /**
     * @brief   Apply Floyd-Steinberg serpentine dithering to the LVGL RGB565
     *          framebuffer and write quantised palette indices to the EPD
     *          framebuffer via writePixelInternal().
     *
     * @param   frameBuffer
     *          Pointer to the LVGL RGB565 render buffer (2 bytes per pixel,
     *          little-endian).
     * @param   width
     *          Buffer width in pixels.
     * @param   height
     *          Buffer height in pixels.
     */
    void ditherFramebuffer(uint8_t *frameBuffer, int width, int height);

  private:
    uint8_t color_index; ///< Index of the last palette entry selected by a map_pixel call.
    Inkplate *_inkplate;
    uint8_t palette_size;
    uint16_t *_palette;
    uint8_t *_paletteIndices;

    /**
     * @brief   Find the nearest palette colour using luminance-weighted Euclidean
     *          distance in RGB space (weights: R×30, G×59, B×11), approximating
     *          human colour sensitivity.
     *
     * @param   _r, _g, _b  Source pixel components (RGB565 precision).
     * @param   palette         Palette array to search.
     * @param   palette_indices Corresponding EPD index array.
     * @param   palette_size    Number of palette entries.
     * @return  RGBTRIPLE of the best-matching palette colour.
     *          Sets color_index to the matched palette position.
     */
    RGBTRIPLE map_pixel_classic(int _r, int _g, int _b, uint16_t *palette, uint8_t *palette_indices,
                                uint8_t palette_size);

    /**
     * @brief   Find the nearest palette colour using unweighted Euclidean distance
     *          in RGB space, with a two-level tie-breaking heuristic that prefers
     *          closer brightness first, then closer saturation. Faster than HSV
     *          matching and produces fewer washed-out results than a bare RGB search.
     *
     * @param   _r, _g, _b  Source pixel components (RGB565 precision).
     * @return  RGBTRIPLE of the best-matching palette colour.
     *          Sets color_index to the matched palette position.
     */
    RGBTRIPLE map_pixel_fast(int _r, int _g, int _b);

    /**
     * @brief   Convert an RGB565 pixel (components at their native precision)
     *          to the HSV colour model.
     *
     * @param   _r, _g, _b  RGB565 components (R: 0–31, G: 0–63, B: 0–31).
     * @param   h           Output hue in degrees [0, 360).
     * @param   s           Output saturation [0, 1].
     * @param   v           Output value (brightness) [0, 1].
     */
    void RGB_to_HSV(int _r, int _g, int _b, float *h, float *s, float *v);

    /**
     * @brief   Compute the perceptual distance between two colours in HSV space.
     *          Hue difference is wrapped to [0, 180] to handle the circular
     *          nature of the hue axis.
     *
     * @param   h1, s1, v1  First colour in HSV.
     * @param   h2, s2, v2  Second colour in HSV.
     * @return  Euclidean distance in normalised HSV space.
     */
    float HSV_distance(float h1, float s1, float v1, float h2, float s2, float v2);

    /**
     * @brief   Find the nearest palette colour by minimising HSV-space distance.
     *          Useful when hue accuracy matters more than absolute RGB accuracy,
     *          though it is slower than map_pixel_fast.
     *
     * @param   _r, _g, _b  Source pixel components (RGB565 precision).
     * @param   palette         Palette array to search.
     * @param   palette_indices Corresponding EPD index array.
     * @param   palette_size    Number of palette entries.
     * @return  RGBTRIPLE of the best-matching palette colour.
     */
    RGBTRIPLE map_pixel_HSV(int _r, int _g, int _b, uint16_t *palette, uint8_t *palette_indices, uint8_t palette_size);

    /**
     * @brief   Clamp an integer value to the inclusive range [_min, _max].
     *
     * @param   _value  Value to clamp.
     * @param   _min    Lower bound.
     * @param   _max    Upper bound.
     * @return  Clamped value as uint8_t.
     */
    uint8_t clampValue(int32_t _value, int32_t _min, int32_t _max);

    /**
     * @brief   Unpack an RGB565 word into its three separate channel values at
     *          native RGB565 precision (R: 5 bits → 0–31, G: 6 bits → 0–63,
     *          B: 5 bits → 0–31).
     *
     * @param   c   Packed RGB565 colour word.
     * @param   r   Output red channel.
     * @param   g   Output green channel.
     * @param   b   Output blue channel.
     */
    void RGB565_to_RGBtriple(uint16_t c, uint8_t *r, uint8_t *g, uint8_t *b);
};

#endif
