/**
 **************************************************
 *
 * @file        colorPalette.ino
 * @brief       Example showing 7 vertical colored rectangles on Inkplate 6COLOR
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit:
 * https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display
Inkplate inkplate;

void setup() {
    // Initialize display in FULL render mode (for color and dithering)
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(0);

    // Set active screen background to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    const int NUM_RECTS = 7;
    const int RECT_WIDTH = E_INK_WIDTH / NUM_RECTS;

    // Colors in RGB565 (16-bit)
    uint16_t colors[NUM_RECTS] = {
        0x0000, // Black   
        0xFFFF, // White   
        0x07E0, // Green   
        0xFFE0, // Yellow  
        0x001F, // Blue    
        0xF800, // Red     
        0xFBE0  // Orange  
    };

    // Create and position vertical rectangles
    for (int i = 0; i < NUM_RECTS; i++) {
        lv_obj_t *rect = lv_obj_create(lv_screen_active());
        lv_obj_set_size(rect, RECT_WIDTH, E_INK_HEIGHT);
        lv_obj_set_pos(rect, i * RECT_WIDTH, 0);

        // Convert RGB565 to LVGL color
        lv_color_t lvcol = lv_color_make(
            (colors[i] >> 11) * 255 / 31,           // R
            ((colors[i] >> 5) & 0x3F) * 255 / 63,  // G
            (colors[i] & 0x1F) * 255 / 31           // B
        );

        // Apply styles
        lv_obj_set_style_bg_color(rect, lvcol, LV_PART_MAIN);
        lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
        lv_obj_set_style_radius(rect, 0, LV_PART_MAIN);
    }

    // Render once
    lv_tick_inc(50);
    lv_timer_handler();

    // Display result
    inkplate.display();
}

void loop() {
    // Static image; no updates needed
}
