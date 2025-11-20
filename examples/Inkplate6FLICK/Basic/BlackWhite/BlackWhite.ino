/**
 **************************************************
 *
 * @file        BlackWhite.ino
 * @brief       Example showing a simple black & white screen split using LVGL on Inkplate 6FLICK
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit https://soldered.com/documentation/inkplate/6flick/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>

// Create Inkplate instance in 1-bit (black & white) mode
Inkplate inkplate(INKPLATE_1BIT);

// Helper functions to get current display width and height from LVGL
static inline int dispW() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_horizontal_resolution(d);
}
static inline int dispH() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_vertical_resolution(d);
}

// Draws the screen divided into black and white halves
void drawBlackWhite() {
  int w = dispW();
  int h = dispH();
  int half = w / 2;

  // Left half — black rectangle
  lv_obj_t *left = lv_obj_create(lv_screen_active());
  lv_obj_remove_flag(left, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(left, half, h);
  lv_obj_set_pos(left, 0, 0);
  lv_obj_set_style_border_width(left, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(left, lv_color_hex(0x000000), LV_PART_MAIN);

  // Right half — white rectangle
  lv_obj_t *right = lv_obj_create(lv_screen_active());
  lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(right, w - half, h);
  lv_obj_set_pos(right, half, 0);
  lv_obj_set_style_border_width(right, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(right, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

void setup() {
  // Initialize Inkplate and LVGL in FULL render mode
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.selectDisplayMode(INKPLATE_1BIT);

  // Set white background and draw both halves
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  drawBlackWhite();

  // Update LVGL and display the rendered frame
  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();
}

void loop() {
  // Nothing to do here; image is static
}