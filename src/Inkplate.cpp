/**
 **************************************************
 *
 * @file        Inkplate.cpp
 * @brief       Core Inkplate class implementation — LVGL initialisation and display control
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Josip Šimun Kuči @ Soldered
 ***************************************************/


#include "Inkplate-LVGL.h"

/**
 * @brief       Inkplate constructor for grayscale/1-bit boards.
 *
 * @param       uint8_t mode
 *              Display mode — INKPLATE_1BIT or INKPLATE_3BIT (board-dependent).
 */
#ifndef USE_COLOR_IMAGE
Inkplate::Inkplate(uint8_t mode)
{
    _mode = mode;
}
#else
/**
 * @brief       Inkplate constructor for colour boards (e.g. Inkplate 13SPECTRA).
 *              No mode argument is needed; the colour format is fixed for these panels.
 */
Inkplate::Inkplate()
{
}
#endif

/**
 *
 * @brief       begin function initializes the EPDdriver as well as the lvgl library
 *
 * @param       lv_display_render_mode_t renderMode - sets what render mode will be used to draw inside the framebuffer
 *              options: LV_DISP_RENDER_MODE_FULL (default), LV_DISP_RENDER_MODE_DIRECT, LV_DISP_RENDER_MODE_PARTIAL
 *
 * @note        LVGL runs single-threaded on Core 0. The user must call
 *              lv_timer_handler() from their loop() (or setup()) to drive
 *              rendering and input processing. LVGL time is sourced from
 *              Arduino millis() via lv_tick_set_cb().
 */
void Inkplate::begin(lv_display_render_mode_t renderMode)
{
    // Check if the initializaton of the library already done.
    // In the case of already initialized library, return form the begin() function to
    // avoid any memory leaks, multiple initializaton of the peripherals etc.
    if (_beginDone == 1)
        return;

    Wire.begin();

    _renderMode = renderMode;

    initLVGL(renderMode);

    // Init low level driver for EPD.
    initDriver(this);

// Forward the display mode to the EPD driver
#ifndef USE_COLOR_IMAGE
    selectDisplayMode(_mode);
#endif

    // Clean frame buffers.
    clearDisplay();

    // Block multiple inits.
    _beginDone = 1;
}

/**
 *
 * @brief       drawPixel function draws a pixel at a specific coordinate on the screen with a specific color value
 *
 * @param       int16_t x0
 *              default position for x, will be changed depending on rotation
 * @param       int16_t y0
 *              default position for y, will be changed depending on rotation
 * @param       uint16_t color
 *              pixel color, in 3bit mode have values in range 0-7
 */
void Inkplate::drawPixel(int16_t x, int16_t y, uint16_t color)
{
    writePixelInternal(x, y, color);
}

/**
 * @brief       setRotation sets the display orientation.
 *
 * @param       uint8_t r
 *              Rotation value 0–3 (0 = default landscape, 1 = 90° CW, 2 = 180°, 3 = 270° CW).
 *              Values are masked to the lower two bits, so any value is accepted safely.
 */
void Inkplate::setRotation(uint8_t r)
{
    _rotation = (r & 3);
    switch (_rotation)
    {
    case 0:
    case 2:
        _width = E_INK_WIDTH;
        _height = E_INK_HEIGHT;
        break;
    case 1:
    case 3:
        _width = E_INK_HEIGHT;
        _height = E_INK_WIDTH;
        break;
    }
}

/**
 * @brief       getRotation returns the currently active display rotation.
 *
 * @return      uint8_t — current rotation value (0–3), as set by setRotation().
 */
uint8_t Inkplate::getRotation()
{
    return _rotation;
}

/**
 * @brief       initLVGL initialises the LVGL library, allocates display buffers, and
 *              registers the flush callback. Called once from begin() before the EPD
 *              driver is initialised. The LVGL tick task is NOT started here — it is
 *              started at the end of begin() after all driver memory is ready.
 *
 * @param       lv_display_render_mode_t renderMode
 *              Render mode passed through to lv_display_set_buffers():
 *              LV_DISP_RENDER_MODE_FULL    — one full-screen buffer (default).
 *              LV_DISP_RENDER_MODE_DIRECT  — two full-screen buffers, direct write.
 *              LV_DISP_RENDER_MODE_PARTIAL — two small strip buffers (PARTIAL_ROWS tall).
 */
void Inkplate::initLVGL(lv_display_render_mode_t renderMode)
{
    // Init the lvgl library itself
    lv_init();

    // Source LVGL time from Arduino millis() — no background tick task needed.
    lv_tick_set_cb(millis);

// Define display resolution
#if !defined(ARDUINO_INKPLATE2) && !defined(ARDUINO_INKPLATE13SPECTRA)
    uint32_t screen_width = E_INK_WIDTH;
    uint32_t screen_height = E_INK_HEIGHT;
#else
    uint32_t screen_width = E_INK_HEIGHT;
    uint32_t screen_height = E_INK_WIDTH;
#endif
    uint32_t buffer_size;
    lv_color_t *buf_1;
    lv_color_t *buf_2;

    if (renderMode == LV_DISPLAY_RENDER_MODE_PARTIAL)
    {
#define PARTIAL_ROWS 16
        buf_1 = (lv_color_t *)heap_caps_malloc(screen_width * PARTIAL_ROWS * (LV_COLOR_DEPTH / 8), MALLOC_CAP_8BIT);
        buf_2 = (lv_color_t *)heap_caps_malloc(screen_width * PARTIAL_ROWS * (LV_COLOR_DEPTH / 8), MALLOC_CAP_8BIT);
        buffer_size = screen_width * PARTIAL_ROWS * (LV_COLOR_DEPTH / 8);
    }
    else
    {
        buf_1 = (lv_color_t *)heap_caps_malloc(screen_width * screen_height * (LV_COLOR_DEPTH / 8), MALLOC_CAP_8BIT);
        buf_2 = (lv_color_t *)heap_caps_malloc(screen_width * screen_height * (LV_COLOR_DEPTH / 8), MALLOC_CAP_8BIT);
        buffer_size = screen_width * screen_height * (LV_COLOR_DEPTH / 8);
    }

    // Create a display driver instance
    disp = lv_display_create(screen_width, screen_height);
    if (disp == NULL)
    {
        Serial.println("ERROR: Failed to create LVGL display!");
        return;
    }

    lv_display_set_default(disp);

// Use RGB565 when using a color display, otherwise opt for 8-bit grayscale
#ifdef USE_COLOR_IMAGE
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_RGB565);
#else
    lv_display_set_color_format(disp, LV_COLOR_FORMAT_L8);
#endif

    // Attach the buffer
    lv_display_set_buffers(disp, buf_1, buf_2, buffer_size, renderMode);

    // Store this Inkplate instance
    lv_display_set_user_data(disp, this);

    // Set flush callback
    lv_display_set_flush_cb(disp, display_flush_callback);

    // Inkplate 2 doesn't have an SD Card reader.
    // Pass 'this' so the FS driver can use the board's own SdFat volume
    // directly instead of relying on the global FatVolume::cwv() pointer.
#ifndef ARDUINO_INKPLATE2
    lv_fs_init_sd(this);
#endif
}

/**
 * @brief       enableDithering enables or disables the dithering algorithm applied
 *              when LVGL flushes pixels into the EPD framebuffer. Dithering improves
 *              the appearance of gradients and images on 1-bit and 3-bit panels at
 *              the cost of a small amount of CPU time per flush.
 *
 * @param       bool state
 *              true  — enable dithering.
 *              false — disable dithering (raw pixel values are used directly).
 */
void Inkplate::enableDithering(bool state)
{
    ditherEnabled = state;
}