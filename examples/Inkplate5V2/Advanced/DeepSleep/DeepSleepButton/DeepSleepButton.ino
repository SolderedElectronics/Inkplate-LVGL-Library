/**
 **************************************************
 *
 * @file        DeepSleepButton.ino
 * @brief       Wake from deep sleep by button (EXT0) or by timer (30 s).
 *
 * For setup instructions and more information about Inkplate 5V2, visit
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * This example demonstrates waking the Inkplate 5V2 either by timer
 * or by a button connected to the EXT0 wakeup pin.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>

// Deep sleep timing
#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP  30

// Keep count across deep sleep cycles
RTC_DATA_ATTR int bootCount = 0;

// Create Inkplate LVGL display object in explicit bit mode
// Use INKPLATE_1BIT for fastest b/w, or INKPLATE_3BIT for grayscale.
Inkplate inkplate(INKPLATE_1BIT);

void setup()
{
  Serial.begin(115200);

  // Initialize display + LVGL in FULL render mode
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);

  // Increment boot counter
  bootCount++;

  // Figure out wake reason
  esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
  const char *reasonText = "Wakeup: Unknown";
  switch (wakeup_reason)
  {
    case ESP_SLEEP_WAKEUP_EXT0:  reasonText = "Wakeup: Button"; break;
    case ESP_SLEEP_WAKEUP_TIMER: reasonText = "Wakeup: Timer";  break;
    default: break;
  }

  // Prepare label text
  char labelText[128];
  snprintf(labelText, sizeof(labelText),
           "Boot count: %d\n%s", bootCount, reasonText);

  // White background
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // Centered label (use default font to avoid config deps)
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, labelText);
  lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  // Render & show
  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();

  Serial.printf("Boot count: %d | %s\n", bootCount, reasonText);

  // Enable wake on timer
  esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

  // Enable wake on button (GPIO 36, active LOW)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

  // Enter deep sleep
  esp_deep_sleep_start();
}

void loop()
{
  // never reached
}
