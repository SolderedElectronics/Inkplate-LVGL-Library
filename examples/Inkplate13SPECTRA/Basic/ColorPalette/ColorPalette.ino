/**
 **************************************************
 *
 * @file        ColorPalette.ino
 * @brief       Example showing 7 vertical colored rectangles on the Inkplate 13 SPECTRA
 *
 * The Inkplate 13 SPECTRA supports full RGB565 color with dithering.
 * This example demonstrates the 7 base colors available on the display.
 *
 * For info on how to quickly get started with Inkplate 13 SPECTRA visit:
 * https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

// Use full RGB565 color space for LVGL
#define USE_COLOR_IMAGE

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display
Inkplate inkplate;

void setup()
{
    /* Initialize display in FULL render mode.
       Dithering is supported in FULL render mode, which allows
       the display to show a wide range of colors. */
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);

    // Set active screen background to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Get the display dimensions from LVGL
    int32_t screen_w = lv_display_get_horizontal_resolution(lv_disp_get_default());
    int32_t screen_h = lv_display_get_vertical_resolution(lv_disp_get_default());

    const int NUM_RECTS = 6;
    const int RECT_WIDTH = screen_w / NUM_RECTS;

    // Colors in RGB888 - the 6 native colors of the Inkplate 13 SPECTRA
    uint32_t colors[NUM_RECTS] = {
        0x000000, // Black
        0xFFFFFF, // White
        0x00FF00, // Green
        0xFFFF00, // Yellow
        0x0000FF, // Blue
        0xFF0000, // Red
    };

    // Create and position vertical rectangles
    for (int i = 0; i < NUM_RECTS; i++)
    {
        lv_obj_t *rect = lv_obj_create(lv_screen_active());
        lv_obj_set_size(rect, RECT_WIDTH, screen_h);
        lv_obj_set_pos(rect, i * RECT_WIDTH, 0);

        // Apply styles
        lv_obj_set_style_bg_color(rect, lv_color_hex(colors[i]), LV_PART_MAIN);
        lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(rect, 0, LV_PART_MAIN);
    }

    // Render once
    lv_tick_inc(50);
    lv_timer_handler();

    // Display result
    inkplate.display();
}

void loop()
{
    // Static image; no updates needed
}
