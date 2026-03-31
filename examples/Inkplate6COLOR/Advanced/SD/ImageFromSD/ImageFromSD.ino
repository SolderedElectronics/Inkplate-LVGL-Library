/**
 **************************************************
 *
 * @file        ImageFromSD.ino
 * @brief       Example showing how to read a jpg file from an SD card and display it on the display
 *              using lvgl, the image can be found in t folder of this example sketch. If you wish to change the image
 *              name, do so in lines 57 and 87
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
// Include the library
#include <Inkplate-LVGL.h>


// Create an instance of the Inkplate object
Inkplate inkplate;

void setup() {
  Serial.begin(115200);
  delay(1000);

  // Initialize display and LVGL in full render mode
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.enableDithering(1);
  inkplate.clearDisplay();
  Serial.println("Display initialized");

  // Initialize SD card
  if (!inkplate.sdCardInit()) {
    Serial.println("SD Card init failed!");
    return;
  }
  Serial.println("SD Card initialized");

  // Note: the LVGL SD filesystem driver is registered automatically by begin().

  // Create the screen and color it white
  lv_obj_t *screen = lv_scr_act();
  lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

  // Create image object
  lv_obj_t *img = lv_image_create(screen);

  // See if the image object was successfully created
  if (img) {
    // Set the source of the image as well as the drive letter defined in lv_conf.h
    lv_image_set_src(img, "S:/cat.jpg");
    lv_obj_center(img);
    Serial.println("Image created and centered");
  }

  // Add status label
  lv_obj_t *label = lv_label_create(screen);
  lv_label_set_text(label, "Image loaded from SD");
  lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 10);

  // Display the image
  inkplate.display();
  Serial.println("Display updated");

  // Turn off the SD card to save power
  inkplate.sdCardSleep();
}


void loop() {

}