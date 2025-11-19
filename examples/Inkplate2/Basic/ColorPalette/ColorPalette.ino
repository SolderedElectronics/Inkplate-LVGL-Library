/**
 **************************************************
 *
 * @file        colorPalette.ino
 * @brief       Example showing 3 vertical colored rectangles on Inkplate 2
 *
 * For info on how to quickly get started with Inkplate 2 visit:
 * https://soldered.com/documentation/inkplate/2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display
Inkplate inkplate;

void setup() {
    // Initialize display in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // Set active screen background to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    const int NUM_RECTS = 3;
    const int RECT_WIDTH = E_INK_HEIGHT / NUM_RECTS;

    // Colors in RGB888
    int colors[NUM_RECTS] = {
        0x000000, // Black   
        0xFFFFFF, // White      
        0xFF0000, // Red     
    };

    // Create and position vertical rectangles
    for (int i = 0; i < NUM_RECTS; i++) {
        lv_obj_t *rect = lv_obj_create(lv_screen_active());
        lv_obj_set_size(rect, RECT_WIDTH, E_INK_HEIGHT);
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

void loop() {
    // Static image; no updates needed
}
