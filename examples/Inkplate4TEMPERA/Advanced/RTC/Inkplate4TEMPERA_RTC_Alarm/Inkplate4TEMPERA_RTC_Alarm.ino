/**
 **************************************************
 *
 * @file        RTCAlarm.ino
 * @brief       Example showing how to set the date and time of the RTC as well as the alarm.
 *              When the alarm is triggered, it displays on the screen along with the timestamp
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit
 * https://soldered.com/documentation/inkplate/6flick/overview/ 
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT); // Inkplate LVGL display instance

// Refresh once per minute
#define REFRESH_DELAY 60000
unsigned long lastRefresh = 0;

// Forward declarations
void updateTimeLabel(lv_obj_t *label, lv_obj_t *alarmLabel);

void setup()
{
    // Initialize Inkplate and LVGL
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.rtc.reset();

    // Set time/date and alarm
    // Set initial time and date
    // 14:30:00
    inkplate.rtc.setTime(14, 30, 0);
    // Wednesday, 12.11.2025.
    inkplate.rtc.setDate(3, 12, 11, 2025);

    // Set the alarm 1 minute from now
    inkplate.rtc.setAlarm(0, 31, 14, 12, 3);

    // Set white background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create main time label
    lv_obj_t *timeLabel = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(timeLabel, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(timeLabel, &lv_font_montserrat_26, 0);
    lv_obj_align(timeLabel, LV_ALIGN_CENTER, 0, -30);

    // Create secondary "ALARM!" label (hidden by default)
    lv_obj_t *alarmLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(alarmLabel, "");
    lv_obj_set_style_text_color(alarmLabel, lv_color_hex(0x00AA00), LV_PART_MAIN); // green
    lv_obj_set_style_text_font(alarmLabel, &lv_font_montserrat_48, 0);
    lv_obj_align(alarmLabel, LV_ALIGN_CENTER, 0, 60);

    // Initial display
    updateTimeLabel(timeLabel, alarmLabel);
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    lastRefresh = millis();
}

void loop()
{
    // Refresh screen once per minute
    if (millis() - lastRefresh > REFRESH_DELAY)
    {
        lv_obj_t *timeLabel = lv_obj_get_child(lv_screen_active(), 0);
        lv_obj_t *alarmLabel = lv_obj_get_child(lv_screen_active(), 1);
        updateTimeLabel(timeLabel, alarmLabel);

        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();

        lastRefresh = millis();
    }
}


// Update the main time label and show alarm status
void updateTimeLabel(lv_obj_t *label, lv_obj_t *alarmLabel)
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

    lv_label_set_text(label, timeText);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, -30);

    // Check and display alarm status
    if (inkplate.rtc.checkAlarmFlag())
    {
        inkplate.rtc.clearAlarmFlag();
        lv_label_set_text(alarmLabel, "ALARM!");
    }
    else
    {
        lv_label_set_text(alarmLabel, "");
    }

    lv_obj_align(alarmLabel, LV_ALIGN_CENTER, 0, 60);
}