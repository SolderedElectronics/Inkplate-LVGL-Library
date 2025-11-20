/**
 **************************************************
 *
 * @file        ImageFromSD.ino
 * @brief       Example showing how to read a jpg file from an SD card and display it
 *              using LVGL. The image must be located in the root of the SD card.
 *              To change the filename, modify lines 57 and 87.
 *
 * For setup instructions and more information about Inkplate 5V2 visit:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>

// Inkplate instance (renamed from `display` → `inkplate`)
Inkplate inkplate(INKPLATE_1BIT);

// LVGL tick task (runs LVGL every 5 ms on core 1)
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

  // Initialize Inkplate + LVGL in full render mode
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.enableDithering(1);
  inkplate.clearDisplay();
  Serial.println("Inkplate initialized");

  // Initialize SD card
  if (!inkplate.sdCardInit()) {
    Serial.println("SD Card init failed!");
    return;
  }
  Serial.println("SD Card initialized");

  // Register LVGL filesystem wrapper
  lv_fs_init_sd();
  Serial.println("LVGL filesystem driver registered");

  // Check if the file exists
  SdFile testFile;
  if (testFile.open("cat.jpg", O_READ)) {
    Serial.printf("File found, size: %lu bytes\n", testFile.fileSize());
    testFile.close();
  } else {
    Serial.println("File NOT found on SD card!");
    return;
  }

  // Start LVGL tick task on core 1
  xTaskCreatePinnedToCore(
      lvgl_task,
      "lvgl_tick",
      16000,
      nullptr,
      2,
      nullptr,
      1
  );

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

  // Allow LVGL to render a few cycles
  for (int i = 0; i < 5; i++) {
    lv_timer_handler();
    delay(10);
  }

  // Turn off SD card for power saving
  inkplate.sdCardSleep();

  // Update e-paper display
  inkplate.display();
  Serial.println("Display updated");
}

void loop() {
  // Nothing needed for this static example
}
