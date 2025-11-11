#include "ditherAlgorithm.h"
#include "Inkplate-LVGL.h"

#ifndef USE_COLOR_IMAGE


void DitherAlgorithm::begin(Inkplate* inkplatePtr)
{
    _inkplate = inkplatePtr;
}


void DitherAlgorithm::ditherFramebuffer(uint8_t* frameBuffer, int width, int height, uint8_t mode)
{
    // mode = 0 → 1-bit (2 levels)
    // mode = 1 → 3-bit (8 levels)
    const int maxLevel = (mode == 0) ? 1 : 7;
    const float scale = 255.0f / maxLevel;

    int16_t *errCurr = (int16_t*)ps_malloc(width * sizeof(int16_t));
    int16_t *errNext = (int16_t*)ps_malloc(width * sizeof(int16_t));
    if (!errCurr || !errNext) return;

    memset(errCurr, 0, width * sizeof(int16_t));

    for (int y = 0; y < height; y++)
    {
        memset(errNext, 0, width * sizeof(int16_t));

        int direction = (y & 1) ? -1 : 1;  // serpentine pattern
        int xStart = (direction == 1) ? 0 : (width - 1);
        int xEnd   = (direction == 1) ? width : -1;

        for (int x = xStart; x != xEnd; x += direction)
        {
            int idx = y * width + x;

            // Apply accumulated error
            int gray = frameBuffer[idx] + errCurr[x];
            if (gray < 0) gray = 0;
            if (gray > 255) gray = 255;

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
                if (quantLevel < 0) quantLevel = 0;
                if (quantLevel > maxLevel) quantLevel = maxLevel;
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

            if (y + 1 < height) {
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