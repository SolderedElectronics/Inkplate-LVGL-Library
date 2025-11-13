/**
 **************************************************
 *
 * @file        RTCAlarmDeepSleep.ino
 * @brief       The following example sets the date and time on the RTC and enables its alarm every 30 seconds,
 *              when the alarm is triggered, the Inkplate device wakes from deep sleep
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>
#include <stdio.h>

Inkplate inkplate(INKPLATE_1BIT); // Create Inkplate LVGL display instance

// Buffer for formatted time string
char timeText[128];

void setup()
{
    // Initialize Inkplate and LVGL in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.rtc.clearAlarmFlag();

    if (!inkplate.rtc.isSet())
    {
        inkplate.rtc.setTime(13, 30, 0);      // 13:30:00
        inkplate.rtc.setDate(3, 12, 11, 2025); // Monday, 12.11.2025
    }

    // Generate time text into char array
    buildCurrentTimeString();

    // Set white background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create LVGL label in the center
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, timeText);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Render label
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    // Set RTC alarm to trigger after 60 seconds
    inkplate.rtc.setAlarmEpoch(inkplate.rtc.getEpoch() + 60, RTC_ALARM_MATCH_DHHMMSS);

    // Enable wake-up on RTC interrupt
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);
    esp_deep_sleep_start(); // Go to deep sleep (never returns)
}

void loop()
{
    // Never executed due to deep sleep
}


// Build formatted time string into global 'timeText' buffer
void buildCurrentTimeString()
{
    inkplate.rtc.getRtcData();

    const char *weekdayNames[] = {
        "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

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