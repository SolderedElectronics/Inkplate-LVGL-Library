/**
 **************************************************
 *
 * @file        SimpleDeepSleep.ino
 * @brief       Put Inkplate into deep sleep, wake every 30 s, and display the wake count.
 *
 *
 * For info on how to quickly get started with Inkplate 6 visit:
 * https://soldered.com/documentation/inkplate/6/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************
 */

#include <Inkplate-LVGL.h>
#include "driver/rtc_io.h"

#define uS_TO_S_FACTOR 1000000ULL  // microseconds → seconds
#define TIME_TO_SLEEP  30          // sleep duration (s)

RTC_DATA_ATTR int wakeCount = 0;   // retained across deep sleep cycles

// Explicit bit depth: use INKPLATE_1BIT (or change to INKPLATE_3BIT)
Inkplate inkplate(INKPLATE_1BIT);

void setup()
{
  Serial.begin(115200);

  // Initialize Inkplate & LVGL (FULL render)
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);

  // Increment persistent counter
  wakeCount++;

  // White background
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // Compose label text
  char labelText[64];
  snprintf(labelText, sizeof(labelText), "Wake count: %d", wakeCount);

  // Centered label (use LV_FONT_DEFAULT to avoid config deps)
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, labelText);
  lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  // Render & show
  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();

  Serial.printf("Device wake count: %d\n", wakeCount);

  // Configure deep sleep wake-up timer
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Enter deep sleep
  esp_deep_sleep_start();
}

void loop()
{
  // Never reached; all logic is in setup()
}
