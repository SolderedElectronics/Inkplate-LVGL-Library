/**
 **************************************************
 *
 * @file        Black_And_White.ino
 * @brief       Example demonstrating 1-bit (black & white) LVGL drawing on Inkplate10
 *
 * For info on how to quickly get started with Inkplate 10 visit:
 * https://soldered.com/documentation/inkplate/10/overview/
 *
 * This example initializes the Inkplate 10 in 1-bit mode and uses LVGL
 * to draw a few simple black & white UI elements.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>   // Include the Inkplate + LVGL integration library

// Create Inkplate instance in 1-bit (black & white) mode
Inkplate inkplate(INKPLATE_1BIT);

// LVGL tick timer (required for internal timekeeping)
esp_timer_handle_t lvgl_tick_timer;

// Timer callback that increments LVGL’s internal time base
void lv_tick_task(void *arg) {
  lv_tick_inc(5); // Increase LVGL tick count by 5 ms
}

void setup() {
  Serial.begin(115200);
  Serial.println("Inkplate LVGL 1-bit (Black & White) drawing example...");

  // Initialize Inkplate and LVGL in FULL render mode
  // FULL mode redraws the entire screen — recommended for static scenes
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.clearDisplay();
  inkplate.enableDithering(false); // Disable dithering (only black/white pixels)

  // Create and start LVGL tick timer (every 5 ms)
  const esp_timer_create_args_t lvgl_tick_args = {
      .callback = &lv_tick_task,
      .arg = nullptr,
      .dispatch_method = ESP_TIMER_TASK,
      .name = "lvgl_tick"};
  esp_timer_create(&lvgl_tick_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, 5000); // 5000 µs = 5 ms period

  // ---- LVGL UI SETUP ----

  // Set background color to white
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);

  // Create a title label
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "1-bit LVGL Demo");
  lv_obj_set_style_text_color(label, lv_color_black(), 0);   // Black text
  lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 40);

  // Draw a simple black rectangle
  lv_obj_t *rect = lv_obj_create(lv_screen_active());
  lv_obj_set_size(rect, 200, 100);
  lv_obj_set_style_bg_color(rect, lv_color_black(), 0);  // Fill color: black
  lv_obj_set_style_border_width(rect, 0, 0);
  lv_obj_align(rect, LV_ALIGN_CENTER, 0, 0);

  // Draw a white circle (by using a transparent background)
  lv_obj_t *circle = lv_obj_create(lv_screen_active());
  lv_obj_set_size(circle, 100, 100);
  lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0); // Make it round
  lv_obj_set_style_bg_color(circle, lv_color_white(), 0); // White fill
  lv_obj_set_style_border_width(circle, 6, 0);
  lv_obj_set_style_border_color(circle, lv_color_black(), 0); // Black outline
  lv_obj_align(circle, LV_ALIGN_BOTTOM_MID, 0, -60);

  // Trigger LVGL to render and show the scene
  lv_timer_handler();
  inkplate.display(); // Perform full refresh (since scene is static)
}

void loop() {
  // Nothing to do — static image stays on e-paper
}
