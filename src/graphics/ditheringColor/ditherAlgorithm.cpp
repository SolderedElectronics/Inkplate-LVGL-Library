#include "ditherAlgorithm.h"
#include "Inkplate-LVGL.h"

#if defined(ARDUINO_INKPLATECOLOR) || defined(ARDUINO_INKPLATE2)

// RGB565 to RGBTRIPLE
void DitherAlgorithm::RGB565_to_RGBtriple(uint16_t c, uint8_t *r, uint8_t *g, uint8_t *b)
{
    *r = (c >> 11) & 0x1F;
    *g = (c >> 5) & 0x3F;
    *b = c & 0x1F;
}

uint16_t color_index;

void DitherAlgorithm::begin(uint16_t *palette, uint8_t *paletteIndices, uint8_t paletteSize, Inkplate *inkplatePtr)
{
    palette_size = paletteSize;

    _palette = (uint16_t *)ps_malloc(palette_size * sizeof(uint16_t));
    _paletteIndices = (uint8_t *)ps_malloc(palette_size * sizeof(uint8_t));
    _inkplate = inkplatePtr;


    memcpy(_palette, palette, palette_size * sizeof(uint16_t));
    memcpy(_paletteIndices, paletteIndices, palette_size * sizeof(uint8_t));
}


// ------------------
// 1. Classic weighted RGB distance (original)
// ------------------
RGBTRIPLE DitherAlgorithm::map_pixel_classic(int _r, int _g, int _b, uint16_t *palette, uint8_t *palette_indices,
                                             uint8_t palette_size)
{
    uint16_t best_color = palette[0];
    uint64_t min_dist = UINT64_MAX;

    for (uint8_t j = 0; j < palette_size; j++)
    {
        uint16_t c = palette[j];
        int c_r = (c >> 11) & 0x1F;
        int c_g = (c >> 5) & 0x3F;
        int c_b = c & 0x1F;

        int dr = _r - c_r;
        int dg = _g - c_g;
        int db = _b - c_b;

        uint64_t dist = ((uint64_t)(dr * dr) * 30) + ((uint64_t)(dg * dg) * 59) + ((uint64_t)(db * db) * 11);

        if (dist < min_dist)
        {
            min_dist = dist;
            best_color = c;
            color_index = j;
        }
    }

    RGBTRIPLE result;
    result.r = (best_color >> 11) & 0x1F;
    result.g = (best_color >> 5) & 0x3F;
    result.b = best_color & 0x1F;
    return result;
}

// ------------------
// 2. Fast unweighted RGB distance
// ------------------
RGBTRIPLE DitherAlgorithm::map_pixel_fast(int _r, int _g, int _b)
{
    uint16_t best_color = _palette[0];
    uint32_t min_dist = UINT32_MAX;
    uint8_t best_idx = 0;

    // Precompute source brightness & saturation-ish
    int srcY = 2 * _r + 5 * _g + _b;                         // cheap luma
    int srcSat = abs(_r - _g) + abs(_g - _b) + abs(_b - _r); // cheap “colorfulness”

    for (uint8_t j = 0; j < palette_size; j++)
    {
        uint16_t c = _palette[j];
        int pr = (c >> 11) & 0x1F;
        int pg = (c >> 5) & 0x3F;
        int pb = c & 0x1F;

        int dr = _r - pr;
        int dg = _g - pg;
        int db = _b - pb;
        uint32_t dist = dr * dr + dg * dg + db * db;

        int palY = 2 * pr + 5 * pg + pb;
        int palSat = abs(pr - pg) + abs(pg - pb) + abs(pb - pr);

        // lower is better
        int brightnessErr = abs(srcY - palY);
        int satDiff = srcSat - palSat; // we like palSat close to or above srcSat

        bool better = false;

        if (dist < min_dist)
        {
            better = true;
        }
        else if (dist == min_dist)
        {
            // Tie-break:
            // 1) pick closer brightness
            int bestY = 2 * ((best_color >> 11) & 0x1F) + 5 * ((best_color >> 5) & 0x3F) + (best_color & 0x1F);
            int bestBrightnessErr = abs(srcY - bestY);

            if (brightnessErr < bestBrightnessErr)
            {
                better = true;
            }
            else if (brightnessErr == bestBrightnessErr)
            {
                // 2) prefer not washing out saturation
                int bestPr = (best_color >> 11) & 0x1F;
                int bestPg = (best_color >> 5) & 0x3F;
                int bestPb = best_color & 0x1F;
                int bestSat = abs(bestPr - bestPg) + abs(bestPg - bestPb) + abs(bestPb - bestPr);

                // If candidate is at least as saturated as best, prefer it.
                if (abs(srcSat - palSat) < abs(srcSat - bestSat) || palSat > bestSat)
                {
                    better = true;
                }
            }
        }

        if (better)
        {
            min_dist = dist;
            best_color = c;
            best_idx = j;
        }
    }

    color_index = best_idx;

    RGBTRIPLE result;
    result.r = (best_color >> 11) & 0x1F;
    result.g = (best_color >> 5) & 0x3F;
    result.b = best_color & 0x1F;
    return result;
}


// ------------------
// 3. HSV nearest
// ------------------
void DitherAlgorithm::RGB_to_HSV(int _r, int _g, int _b, float *h, float *s, float *v)
{
    float r = _r / 31.0f;
    float g = _g / 63.0f;
    float b = _b / 31.0f;

    float max = fmaxf(r, fmaxf(g, b));
    float min = fminf(r, fminf(g, b));
    *v = max;

    float delta = max - min;
    if (max == 0.0f)
    {
        *s = 0.0f;
        *h = 0.0f;
        return;
    }
    *s = delta / max;

    if (delta == 0.0f)
    {
        *h = 0.0f;
    }
    else if (max == r)
    {
        *h = 60.0f * fmodf(((g - b) / delta), 6.0f);
    }
    else if (max == g)
    {
        *h = 60.0f * (((b - r) / delta) + 2.0f);
    }
    else
    { // max == b
        *h = 60.0f * (((r - g) / delta) + 4.0f);
    }

    if (*h < 0.0f)
        *h += 360.0f;
}

float DitherAlgorithm::HSV_distance(float h1, float s1, float v1, float h2, float s2, float v2)
{
    float dh = fminf(fabsf(h1 - h2), 360.0f - fabsf(h1 - h2)) / 180.0f;
    float ds = s1 - s2;
    float dv = v1 - v2;
    return sqrtf(dh * dh + ds * ds + dv * dv);
}

RGBTRIPLE DitherAlgorithm::map_pixel_HSV(int _r, int _g, int _b, uint16_t *palette, uint8_t *palette_indices,
                                         uint8_t palette_size)
{
    float ph, ps, pv;
    RGB_to_HSV(_r, _g, _b, &ph, &ps, &pv);

    uint16_t best_color = palette[0];
    float best_dist = FLT_MAX;

    for (uint8_t j = 0; j < palette_size; j++)
    {
        RGBTRIPLE palRgb;
        int _palr = (palette[j] >> 11) & 0x1F;
        int _palg = (palette[j] >> 5) & 0x3F;
        int _palb = palette[j] & 0x1F;

        float h, s, v;
        RGB_to_HSV(_palr, _palg, _palb, &h, &s, &v);
        float dist = HSV_distance(ph, ps, pv, h, s, v);
        if (dist < best_dist)
        {
            best_dist = dist;
            best_color = palette[j];
        }
    }

    RGBTRIPLE result;
    result.r = (best_color >> 11) & 0x1F;
    result.g = (best_color >> 5) & 0x3F;
    result.b = best_color & 0x1F;
    return result;
}

uint8_t DitherAlgorithm::clampValue(int32_t _value, int32_t _min, int32_t _max)
{
    if (_value > _max)
        _value = _max;
    if (_value < _min)
        _value = _min;
    return _value;
}

void DitherAlgorithm::ditherFramebuffer(uint8_t *frameBuffer, int width, int height)
{

    // Allocate a 2D array for pixels
    RGBTRIPLE **pixels = (RGBTRIPLE **)ps_malloc(height * sizeof(RGBTRIPLE *));
    if (!pixels)
    {
        return;
    }

    for (int y = 0; y < height; y++)
    {
        pixels[y] = (RGBTRIPLE *)ps_malloc(width * sizeof(RGBTRIPLE));
        if (!pixels[y])
        {
            for (int i = 0; i < y; i++)
                free(pixels[i]);
            free(pixels);
            return;
        }
    }

    // ---------------------------
    // LOAD frameBuffer → pixels[][]
    // ---------------------------
    // frameBuffer is RGB565 (2 bytes per pixel)
    for (int y = 0; y < height; y++)
    {
        for (int x = 0; x < width; x++)
        {
            int index = (y * width + x) * 2;

            uint16_t rgb565 = (frameBuffer[index + 1] << 8) | frameBuffer[index];


            RGBTRIPLE px;
            px.r = (rgb565 >> 11) & 0x1F;
            px.g = (rgb565 >> 5) & 0x3F;
            px.b = rgb565 & 0x1F;

            pixels[y][x] = px;
        }
    }


    // Prepare dithering
    // int16_t errCurrR[width];
    int16_t *errCurrR = (int16_t *)ps_malloc(width * sizeof(int16_t));
    // int16_t errCurrG[width];
    int16_t *errCurrG = (int16_t *)ps_malloc(width * sizeof(int16_t));
    // int16_t errCurrB[width];
    int16_t *errCurrB = (int16_t *)ps_malloc(width * sizeof(int16_t));

    // int16_t errNextR[width];
    // int16_t errNextG[width];
    // int16_t errNextB[width];

    int16_t *errNextR = (int16_t *)ps_malloc(width * sizeof(int16_t));
    // int16_t errCurrG[width];
    int16_t *errNextG = (int16_t *)ps_malloc(width * sizeof(int16_t));
    // int16_t errCurrB[width];
    int16_t *errNextB = (int16_t *)ps_malloc(width * sizeof(int16_t));


    memset(errCurrR, 0, width * sizeof(int16_t));
    memset(errCurrG, 0, width * sizeof(int16_t));
    memset(errCurrB, 0, width * sizeof(int16_t));


    for (int y = 0; y < height; y++)
    {
        memset(errNextR, 0, width * sizeof(int16_t));
        memset(errNextG, 0, width * sizeof(int16_t));
        memset(errNextB, 0, width * sizeof(int16_t));

        int direction = (y & 1) ? -1 : 1; // even rows → L→R, odd rows → R→L
        int xStart = (direction == 1) ? 0 : (width - 1);
        int xEnd = (direction == 1) ? width : -1;

        for (int x = xStart; x != xEnd; x += direction)
        {
            // Apply accumulated error
            int r = pixels[y][x].r + errCurrR[x];
            int g = pixels[y][x].g + errCurrG[x];
            int b = pixels[y][x].b + errCurrB[x];

            // Clamp to RGB565 range
            r = clampValue(r, 0, 0x1F);
            g = clampValue(g, 0, 0x3F);
            b = clampValue(b, 0, 0x1F);

            RGBTRIPLE quant = map_pixel_fast(r, g, b);

            uint16_t colorPicked = color_index;
            _inkplate->writePixelInternal(x, y, colorPicked);

            int errorRed = r - quant.r;
            int errorGreen = g - quant.g;
            int errorBlue = b - quant.b;

            pixels[y][x] = quant;

            // ---- FIXED: Correct serpentine error distribution ----
            // Floyd-Steinberg pattern (relative to scan direction):
            //        X   7/16  <- next pixel in scan
            //  3/16 5/16 1/16
            //   ^
            //   behind scan

            // Calculate next pixel index (in scan direction, not yet processed)
            int xNext = x + direction;

            // Horizontal error (7/16) - goes to NEXT unprocessed pixel
            if (xNext >= 0 && xNext < width)
            {
                errCurrR[xNext] += (errorRed * 7) / 16;
                errCurrG[xNext] += (errorGreen * 7) / 16;
                errCurrB[xNext] += (errorBlue * 7) / 16;
            }

            if (y + 1 < height)
            {
                // For next row, the pattern depends on direction:
                // Going L→R (dir=1):  [x-1]  [x]  [x+1]
                //                      3/16  5/16  1/16
                // Going R→L (dir=-1): [x+1]  [x]  [x-1]
                //                      3/16  5/16  1/16

                int xBehind = x - direction; // Behind in scan direction
                int xAhead = x + direction;  // Ahead in scan direction

                // Behind (3/16)
                if (xBehind >= 0 && xBehind < width)
                {
                    errNextR[xBehind] += (errorRed * 3) / 16;
                    errNextG[xBehind] += (errorGreen * 3) / 16;
                    errNextB[xBehind] += (errorBlue * 3) / 16;
                }

                // Below current (5/16)
                errNextR[x] += (errorRed * 5) / 16;
                errNextG[x] += (errorGreen * 5) / 16;
                errNextB[x] += (errorBlue * 5) / 16;

                // Ahead (1/16)
                if (xAhead >= 0 && xAhead < width)
                {
                    errNextR[xAhead] += (errorRed * 1) / 16;
                    errNextG[xAhead] += (errorGreen * 1) / 16;
                    errNextB[xAhead] += (errorBlue * 1) / 16;
                }
            }
        }

        // Move next-row errors down
        memcpy(errCurrR, errNextR, width * sizeof(int16_t));
        memcpy(errCurrG, errNextG, width * sizeof(int16_t));
        memcpy(errCurrB, errNextB, width * sizeof(int16_t));
    }

    for (int y = 0; y < height; y++)
    {
        free(pixels[y]);
    }
    free(pixels);
    return;
}

#endif