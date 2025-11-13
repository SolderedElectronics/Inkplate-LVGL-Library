/**
 **************************************************
 *
 * @file        ImageFromBuffer.ino
 * @brief       Example showing how to display a JPG image located in a buffer onto the display
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 * 
 * The image array data was generated using the following tool: https://notisrac.github.io/FileToCArray/
 * The following options were used: - Code format: Hex(0x00)
 *                                  - Palette mod: 24bit RGB
 *                                  - Multi line
 *                                  - Separate bytes of pixels
 *                                  - rest of the options can stay default
 * Save the generated file and copy it to this example folder 
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>

// Include the image data
#include "cat.h"
// Create an instance of the Inkplate object
Inkplate display;

lv_obj_t *screen;

// LVGL image descriptor
const lv_image_dsc_t my_image = {
    .header = {
        .cf = LV_COLOR_FORMAT_RGB888,   // RGB888 color format
        .w = CAT_WIDTH,
        .h = CAT_HEIGHT,
    },
    .data_size = CAT_WIDTH*CAT_HEIGHT*3,
    .data = cat,
};

// Create an lvgl task which will tick the lvgl timer every 5 ms
// and handle any animations needed
void lvgl_task(void *arg) {
  for (;;) {
    lv_tick_inc(5);
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));  
  }
}

void setup() {
  Serial.begin(115200);
  delay(1000);
  Serial.println("Starting LVGL image example...");

  // Initialize the display as well as lvgl in full render mode
  display.begin(LV_DISP_RENDER_MODE_FULL);

  // Enables dithering of the whole lvgl buffer
  // Note: Dithering is only supported in full render mode
  display.enableDithering(1);

  // Create LVGL task on core 1 to run independently from the rest of the sketch
  xTaskCreatePinnedToCore(
      lvgl_task, // Function which will be pinned 
      "lvgl_tick", // Symbolic name
      16000,  // Stack depth       
      nullptr, // No parameters
      2, // Priority
      nullptr, // No buffer, it will be allocated dynamically
      1 // core used
  );

  delay(100);

  // Create screen
  screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  // Create LVGL image object
  lv_obj_t *img = lv_image_create(screen);
  lv_image_set_src(img, &my_image);
  lv_obj_center(img);

  // Force initial render
  for (int i = 0; i < 5; i++) {
    lv_timer_handler();
    delay(10);
  }

  display.display();
  Serial.println("Display updated with L8 image");
}

void loop() {
  // Nothing required
}