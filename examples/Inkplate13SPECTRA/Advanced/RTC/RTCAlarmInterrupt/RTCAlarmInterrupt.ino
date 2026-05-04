/**
 **************************************************
 *
 * @file        RTCAlarmInterrupt.ino
 * @brief       Example showing how to set up the date and time as well as an alarm
 *              on the RTC using Unix epoch time. When the alarm is triggered, an
 *              interrupt fires on GPIO 18 and "ALARM!" is displayed on screen.
 *
 * For info on how to quickly get started with Inkplate 13SPECTRA visit
 * https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

// Use full RGB565 color space for LVGL
#define USE_COLOR_IMAGE

#include <Inkplate-LVGL.h>

Inkplate inkplate;
volatile bool alarmFlag = false;

// Interrupt Service Routine
void IRAM_ATTR alarmISR()
{
    alarmFlag = true;
}

void updateTimeLabel(lv_obj_t *timeLabel, lv_obj_t *alarmLabel);

void setup()
{
    Serial.begin(115200);

    // RTC interrupt is on GPIO 18 on Inkplate 13SPECTRA
    pinMode(18, INPUT_PULLUP);

    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);
    inkplate.rtc.reset();

    // Set RTC to a known epoch and set an alarm 60s later
    inkplate.rtc.setEpoch(1762957188);
    inkplate.rtc.setAlarmEpoch(inkplate.rtc.getEpoch() + 60, RTC_ALARM_MATCH_DHHMMSS);

    // Attach interrupt on GPIO 18 (RTC INT, active low)
    attachInterrupt(18, alarmISR, FALLING);

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t *timeLabel = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_26, 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -30);

    lv_obj_t *alarmLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(alarmLabel, "");
    lv_obj_set_style_text_color(alarmLabel, lv_color_hex(0xFF0000), LV_PART_MAIN); // red
    lv_obj_set_style_text_font(alarmLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(alarmLabel, LV_ALIGN_CENTER, 0, 60);

    updateTimeLabel(timeLabel, alarmLabel);
    lv_timer_handler();
    inkplate.display();
}

void loop()
{
    static unsigned long lastUpdate = 0;

    if (millis() - lastUpdate > 60000)
    {
        lv_obj_t *timeLabel  = lv_obj_get_child(lv_screen_active(), 0);
        lv_obj_t *alarmLabel = lv_obj_get_child(lv_screen_active(), 1);

        updateTimeLabel(timeLabel, alarmLabel);
        lv_timer_handler();
        inkplate.display();

        lastUpdate = millis();
    }
}

void updateTimeLabel(lv_obj_t *timeLabel, lv_obj_t *alarmLabel)
{
    inkplate.rtc.getRtcData();

    const char *wdayNames[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

    char timeText[128];
    snprintf(timeText, sizeof(timeText),
             "%02d:%02d:%02d\n%s, %02d/%02d/%04d",
             inkplate.rtc.getHour(), inkplate.rtc.getMinute(), inkplate.rtc.getSecond(),
             wdayNames[inkplate.rtc.getWeekday()],
             inkplate.rtc.getDay(), inkplate.rtc.getMonth(), inkplate.rtc.getYear());

    lv_label_set_text(timeLabel, timeText);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -30);

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
