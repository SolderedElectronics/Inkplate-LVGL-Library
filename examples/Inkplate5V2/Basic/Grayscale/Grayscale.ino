/**
 **************************************************
 *
 * @file        Grayscale.ino
 * @brief       Example showing 8 shades of gray using LVGL on Inkplate 5V2 (3-bit mode)
 *
 * For setup instructions and more information about Inkplate 5V2, visit:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * This example displays 8 vertical grayscale bars using LVGL.
 *
 * @authors     Soldered
 * @date        November 2025
 **************************************************
 */

#include <Inkplate-LVGL.h>

// Create Inkplate instance in 3-bit (grayscale) mode
Inkplate inkplate(INKPLATE_3BIT);

// Helper functions to get display dimensions from LVGL
static inline int dispW() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_horizontal_resolution(d);
}
static inline int dispH() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_vertical_resolution(d);
}

// Draw 8 vertical bars representing 8 shades of gray (0–7)
void drawGrayscaleBars() {
  int w = dispW();
  int h = dispH();
  int colW = w / 8;

  // Create gray bars
  for (int i = 0; i < 8; i++) {
    uint8_t shade = i * 36; // convert 0–7 to 0–255 grayscale steps
    lv_obj_t *rect = lv_obj_create(lv_screen_active());
    lv_obj_remove_flag(rect, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(rect, colW, h);
    lv_obj_set_pos(rect, i * colW, 0);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect, lv_color_make(shade, shade, shade), LV_PART_MAIN);
  }

  // Add small numeric labels (0–7) above each bar
  for (int i = 0; i < 8; i++) {
    char txt[4];
    sprintf(txt, "%d", i);
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, i * colW + colW / 2 - 5, 10);
  }
}

void setup() {
  // Initialize Inkplate and LVGL in FULL render mode (required for dithering)
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.selectDisplayMode(INKPLATE_3BIT);

  // Clear the background and draw the bars
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  drawGrayscaleBars();

  // Render the layout and show it on the e-paper display
  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();
}

void loop() {
  // Nothing to do here; the image is static
}
