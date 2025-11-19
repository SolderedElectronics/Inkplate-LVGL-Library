/**
 **************************************************
 *
 * @file        RTCAlarmInterrupt.ino
 * @brief       Example showing how to set up the date and time as well as an alarm on the RTC in Unix epoch time.
 *              When an alarm is triggered, an interrupt is sent over pin 39.
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit
 * https://soldered.com/documentation/inkplate/6flick/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT);  // Inkplate LVGL display instance
volatile bool alarmFlag = false;   // Flag set by interrupt

// Interrupt Service Routine (ISR)
void IRAM_ATTR alarmISR()
{
  alarmFlag = true;
}

// Forward declaration
void updateTimeLabel(lv_obj_t *timeLabel, lv_obj_t *alarmLabel);

void setup()
{
    Serial.begin(115200);
    pinMode(39, INPUT_PULLUP); // RTC INT pin (GPIO39)

    // Initialize Inkplate in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.rtc.reset();

    // Set RTC to a known epoch and set an alarm 60s later
    inkplate.rtc.setEpoch(1762957188);
    inkplate.rtc.setAlarmEpoch(inkplate.rtc.getEpoch() + 60, RTC_ALARM_MATCH_DHHMMSS);

    // Attach interrupt
    attachInterrupt(39, alarmISR, FALLING);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create main time label
    lv_obj_t *timeLabel = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_26, 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -30);

    // Create alarm label (hidden until alarm triggers)
    lv_obj_t *alarmLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(alarmLabel, "");
    lv_obj_set_style_text_color(alarmLabel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(alarmLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(alarmLabel, LV_ALIGN_CENTER, 0, 60);

    // Initial display
    updateTimeLabel(timeLabel, alarmLabel);
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}

void loop()
{
    static unsigned long lastUpdate = 0;

    // Refresh every minute
    if (millis() - lastUpdate > 60000)
    {
        lv_obj_t *timeLabel = lv_obj_get_child(lv_screen_active(), 0);
        lv_obj_t *alarmLabel = lv_obj_get_child(lv_screen_active(), 1);

        updateTimeLabel(timeLabel, alarmLabel);
        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();

        lastUpdate = millis();
    }
}


// Update label text and check alarm status
void updateTimeLabel(lv_obj_t *timeLabel, lv_obj_t *alarmLabel)
{
  inkplate.rtc.getRtcData();

  uint8_t hour = inkplate.rtc.getHour();
  uint8_t minute = inkplate.rtc.getMinute();
  uint8_t second = inkplate.rtc.getSecond();
  uint8_t day = inkplate.rtc.getDay();
  uint8_t weekday = inkplate.rtc.getWeekday();
  uint8_t month = inkplate.rtc.getMonth();
  uint16_t year = inkplate.rtc.getYear();

  const char *wdayNames[] = {
      "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

  char timeText[128];
  snprintf(timeText, sizeof(timeText),
            "%02d:%02d:%02d\n%s, %02d/%02d/%04d",
            hour, minute, second, wdayNames[weekday], day, month, year);

  lv_label_set_text(timeLabel, timeText);
  lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -30);

  // Handle alarm interrupt flag
  if (alarmFlag)
  {
      alarmFlag = false;
      inkplate.rtc.clearAlarmFlag();
      lv_label_set_text(alarmLabel, "ALARM!");
  }
  else
  {
      lv_label_set_text(alarmLabel, "");
  }

  lv_obj_align(alarmLabel, LV_ALIGN_CENTER, 0, 60);
}