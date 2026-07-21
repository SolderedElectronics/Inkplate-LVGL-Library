/**
 **************************************************
 *
 * @file        ditherAlgorithm.cpp
 * @brief       Floyd-Steinberg serpentine dithering implementation for
 *              grayscale EPD panels (1-bit and 3-bit display modes).
 *
 * @copyright   GNU Lesser General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#include "ditherAlgorithm.h"
#include "Inkplate-LVGL.h"

#if !defined(ARDUINO_INKPLATECOLOR) && !defined(ARDUINO_INKPLATE2) && !defined(ARDUINO_INKPLATE13SPECTRA) &&           \
    !defined(ARDUINO_ESP32S3_DEV)


/**
 * @brief   Store the Inkplate instance pointer for later use by ditherFramebuffer().
 *
 * @param   inkplatePtr
 *          Pointer to the owning Inkplate instance.
 */
void DitherAlgorithm::begin(Inkplate *inkplatePtr)
{
    _inkplate = inkplatePtr;
}


/**
 * @brief   Apply Floyd-Steinberg serpentine dithering to the LVGL L8 framebuffer
 *          and write the result directly into the EPD framebuffer via writePixelInternal().
 *
 *          For each pixel, the accumulated diffusion error is added to the raw
 *          luminance value and the result is quantised to the nearest available
 *          display level. The quantisation error is then spread to unprocessed
 *          neighbours using the Floyd-Steinberg kernel:
 *
 *                       [curr] [7/16]
 *              [3/16] [5/16] [1/16]
 *
 *          The scan direction alternates every row (serpentine scan) to reduce
 *          directional banding artefacts. On odd rows the kernel is mirrored
 *          horizontally.
 *
 *          All arithmetic is integer-only to avoid floating-point rounding
 *          errors that would cause systematic shade mismatches.
 *
 * @param   frameBuffer  Pointer to the LVGL L8 (8-bit grayscale) render buffer.
 * @param   width        Buffer width in pixels.
 * @param   height       Buffer height in pixels.
 * @param   mode         0 = 1-bit (black and white), 1 = 3-bit (8 grayscale levels).
 */
void DitherAlgorithm::ditherFramebuffer(uint8_t *frameBuffer, int width, int height, uint8_t mode)
{
    // Allocate two row-sized error buffers in PSRAM. errCurr holds the errors
    // being consumed on the current row; errNext accumulates errors for the row below.
    int16_t *errCurr = (int16_t *)ps_malloc(width * sizeof(int16_t));
    int16_t *errNext = (int16_t *)ps_malloc(width * sizeof(int16_t));
    if (!errCurr || !errNext)
    {
        free(errCurr);
        free(errNext);
        return;
    }
    memset(errCurr, 0, width * sizeof(int16_t));

    for (int y = 0; y < height; y++)
    {
        memset(errNext, 0, width * sizeof(int16_t));

        // Alternate scan direction each row for serpentine dithering
        int dir = (y & 1) ? -1 : 1;
        int xStart = (dir > 0) ? 0 : width - 1;
        int xEnd = (dir > 0) ? width : -1;

        for (int x = xStart; x != xEnd; x += dir)
        {
            // Add the accumulated diffusion error to the raw luminance and clamp to [0, 255]
            int gray = (int)frameBuffer[y * width + x] + errCurr[x];
            if (gray < 0)
                gray = 0;
            if (gray > 255)
                gray = 255;

            int quantLevel, quantGray;

            if (mode == 0)
            {
                // 1-bit mode: simple threshold at 128
                quantLevel = (gray >= 128) ? 1 : 0;
                quantGray = quantLevel ? 255 : 0;
                _inkplate->writePixelInternal(x, y, !quantLevel);
            }
            else
            {
                // 3-bit mode: round gray (0-255) to the nearest of 8 EPD levels (0-7).
                // Using integer rounding: (gray * 7 + 127) / 255 gives the nearest level
                // with no floating-point error and exact values at both endpoints.
                quantLevel = (gray * 7 + 127) / 255;
                if (quantLevel > 7)
                    quantLevel = 7;

                // Reconstruct the brightness that level actually represents on the display.
                // (quantLevel * 255 + 3) / 7 rounds to nearest, ensuring level 7 maps
                // exactly to 255 (no off-by-one that would leave a persistent residual error).
                quantGray = (quantLevel * 255 + 3) / 7;

                _inkplate->writePixelInternal(x, y, quantLevel);
            }

            // Quantisation error: positive means the chosen level was darker than the input
            int err = gray - quantGray;

            // Distribute the error to unprocessed neighbours using the Floyd-Steinberg kernel.
            // dir determines which neighbour is "forward" so the kernel mirrors on odd rows.
            int xFwd = x + dir;
            int xBack = x - dir;

            if (xFwd >= 0 && xFwd < width)
                errCurr[xFwd] += (err * 7) / 16;

            if (y + 1 < height)
            {
                if (xBack >= 0 && xBack < width)
                    errNext[xBack] += (err * 3) / 16;

                errNext[x] += (err * 5) / 16;

                if (xFwd >= 0 && xFwd < width)
                    errNext[xFwd] += (err * 1) / 16;
            }
        }

        // Swap the row buffers: the next row's accumulated errors become the current ones
        int16_t *tmp = errCurr;
        errCurr = errNext;
        errNext = tmp;
    }

    free(errCurr);
    free(errNext);
}


#endif
