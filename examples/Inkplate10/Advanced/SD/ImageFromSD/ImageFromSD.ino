/**
 **************************************************
 *
 * @file        ImageFromSD.ino
 * @brief       Example showing how to read a jpg file from an SD card and display it
 *              using LVGL. The image must be located in the root of the SD card.
 *              To change the filename, modify the open() call and lv_image_set_src().
 *
 * For info on how to quickly get started with Inkplate 10 visit:
 * https://soldered.com/documentation/inkplate/10/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>

// Inkplate instance (renamed from `display` → `inkplate`)
Inkplate inkplate(INKPLATE_1BIT);

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize Inkplate + LVGL in full render mode
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.enableDithering(1);
  inkplate.clearDisplay();
  Serial.println("Inkplate 10 initialized");

  // Initialize SD card
  if (!inkplate.sdCardInit()) {
    Serial.println("SD Card init failed!");
    return;
  }
  Serial.println("SD Card initialized");

  // Note: the LVGL SD filesystem driver is registered automatically by begin().

  // Screen setup
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  // Create LVGL image object
  lv_obj_t *img = lv_image_create(screen);
  if (img) {
    lv_image_set_src(img, "S:/cat.jpg");   // Set SD image source
    lv_obj_center(img);
    Serial.println("Image loaded & centered");
  }

  // Status label
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Image loaded from SD");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

  // Update e-paper display
  inkplate.display();
  Serial.println("Display updated");

  // Turn off SD card for power saving
  inkplate.sdCardSleep();
}

void loop() {
  // Nothing needed for this static example
}
