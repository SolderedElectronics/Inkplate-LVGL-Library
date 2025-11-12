/**
 **************************************************
 *
 * @file        RTCAlarmDeepSleep.ino
 * @brief       Demonstrates how to use the built-in RTC to wake Inkplate 6 from deep sleep
 *              every 60 seconds using an RTC alarm interrupt.
 *
 *              The example sets the initial date/time if not already stored,
 *              displays the current time on the e-paper, and enters deep sleep.
 *              When the RTC alarm triggers, the device wakes up and refreshes the time.
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
#include <stdio.h>

// Explicit bit depth (1-bit BW or 3-bit grayscale)
Inkplate inkplate(INKPLATE_1BIT);

// Buffer for formatted time string
static char timeText[128];

void buildCurrentTimeString();

void setup()
{
  // Init Inkplate + LVGL
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.rtc.clearAlarmFlag();

  // If RTC not set, initialize it
  if (!inkplate.rtc.isSet())
  {
    inkplate.rtc.setTime(13, 30, 0);       // 13:30:00
    inkplate.rtc.setDate(3, 12, 11, 2025); // Monday, 12.11.2025
  }

  // Generate readable time text
  buildCurrentTimeString();

  // White background
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // Create centered label
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, timeText);
  lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, LV_FONT_DEFAULT, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

  // Render and display
  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();

  // Schedule RTC alarm 60 seconds ahead
  inkplate.rtc.setAlarmEpoch(inkplate.rtc.getEpoch() + 60, RTC_ALARM_MATCH_DHHMMSS);

  // Enable RTC interrupt as wake-up source (GPIO 39)
  esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);

  // Enter deep sleep
  esp_deep_sleep_start();
}

void loop()
{
  // Never reached
}

// Format RTC time into global buffer
void buildCurrentTimeString()
{
  inkplate.rtc.getRtcData();

  const char *weekdayNames[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
  };

  uint8_t wd = inkplate.rtc.getWeekday();
  uint8_t day = inkplate.rtc.getDay();
  uint8_t month = inkplate.rtc.getMonth();
  uint16_t year = inkplate.rtc.getYear();
  uint8_t hour = inkplate.rtc.getHour();
  uint8_t minute = inkplate.rtc.getMinute();
  uint8_t second = inkplate.rtc.getSecond();

  snprintf(timeText, sizeof(timeText),
           "%s, %02d.%02d.%04d\n%02d:%02d:%02d",
           weekdayNames[wd], day, month, year, hour, minute, second);
}
