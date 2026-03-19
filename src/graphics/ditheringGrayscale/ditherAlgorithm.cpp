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

#if !defined(ARDUINO_INKPLATECOLOR) && !defined(ARDUINO_INKPLATE2) && !defined(ARDUINO_INKPLATE13SPECTRA)


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
 *          Algorithm overview:
 *            1. For each pixel, add the accumulated diffusion error to its raw value.
 *            2. Quantise the corrected value to the nearest available display level:
 *                 - mode 0 (1-bit): threshold at 128 → black (0) or white (1).
 *                 - mode 1 (3-bit): round to nearest of 8 levels (0–7).
 *            3. Compute the quantisation error (input − quantised output).
 *            4. Distribute the error to unprocessed neighbours using the
 *               Floyd-Steinberg kernel:
 *                           [  X  ] [7/16]
 *                   [3/16]  [5/16]  [1/16]
 *            5. Alternate scan direction each row (serpentine) to reduce
 *               directional banding artefacts.
 *
 *          Two PSRAM row-buffers (errCurr / errNext) hold the per-channel
 *          accumulated errors; they are swapped at the end of each row.
 *
 * @param   frameBuffer
 *          Pointer to the LVGL L8 (8-bit grayscale) render buffer.
 * @param   width
 *          Buffer width in pixels.
 * @param   height
 *          Buffer height in pixels.
 * @param   mode
 *          0 = 1-bit (black & white only), 1 = 3-bit (8 grayscale levels).
 */
void DitherAlgorithm::ditherFramebuffer(uint8_t *frameBuffer, int width, int height, uint8_t mode)
{
    const int maxLevel = (mode == 0) ? 1 : 7;
    const float scale = 255.0f / maxLevel;

    int16_t *errCurr = (int16_t *)ps_malloc(width * sizeof(int16_t));
    int16_t *errNext = (int16_t *)ps_malloc(width * sizeof(int16_t));
    if (!errCurr || !errNext)
        return;

    memset(errCurr, 0, width * sizeof(int16_t));

    for (int y = 0; y < height; y++)
    {
        memset(errNext, 0, width * sizeof(int16_t));

        int direction = (y & 1) ? -1 : 1; // serpentine pattern
        int xStart = (direction == 1) ? 0 : (width - 1);
        int xEnd = (direction == 1) ? width : -1;

        for (int x = xStart; x != xEnd; x += direction)
        {
            int idx = y * width + x;

            // Apply accumulated error
            int gray = frameBuffer[idx] + errCurr[x];
            if (gray < 0)
                gray = 0;
            if (gray > 255)
                gray = 255;

            // Quantize depending on mode
            int quantLevel;
            if (mode == 0)
            {
                // --- 1-bit mode (black & white only) ---
                quantLevel = (gray >= 128) ? 1 : 0;
                _inkplate->writePixelInternal(x, y, !quantLevel);
            }
            else
            {
                // --- 3-bit mode (8 grayscale levels) ---
                quantLevel = (int)roundf(gray / scale);
                if (quantLevel < 0)
                    quantLevel = 0;
                if (quantLevel > maxLevel)
                    quantLevel = maxLevel;
                // Write quantized level (0–1 or 0–7)
                _inkplate->writePixelInternal(x, y, quantLevel);
            }


            // Reconstruct quantized brightness for error calculation
            int quantGray = (int)(quantLevel * scale);

            // Diffusion error
            int error = gray - quantGray;

            // Floyd–Steinberg diffusion
            int xNext = x + direction;
            if (xNext >= 0 && xNext < width)
                errCurr[xNext] += (error * 7) / 16;

            if (y + 1 < height)
            {
                int xBehind = x - direction;
                int xAhead = x + direction;

                if (xBehind >= 0 && xBehind < width)
                    errNext[xBehind] += (error * 3) / 16;

                errNext[x] += (error * 5) / 16;

                if (xAhead >= 0 && xAhead < width)
                    errNext[xAhead] += (error * 1) / 16;
            }
        }

        memcpy(errCurr, errNext, width * sizeof(int16_t));
    }

    free(errCurr);
    free(errNext);
}


#endif
