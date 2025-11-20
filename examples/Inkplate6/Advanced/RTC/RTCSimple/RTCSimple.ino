/**
 **************************************************
 *
 * @file        RTCSimple.ino
 * @brief       Example showing the functionalities of the onboard RTC by setting its date and time and refreshing the screen every minute
 *
 * For info on how to quickly get started with Inkplate 6 visit
 * https://soldered.com/documentation/inkplate/6/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT); // Inkplate LVGL display instance

#define REFRESH_DELAY 60000 // 1 minute refresh interval (ms)
unsigned long lastRefresh = 0;

// Forward declarations
void updateTimeLabel(lv_obj_t *label);

void setup()
{
    // Initialize Inkplate with LVGL
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.rtc.reset(); // Reset RTC if any data present

    // Set initial time and date
    // 14:30:00
    inkplate.rtc.setTime(14, 30, 0);
    // Wednesday, 12.11.2025.
    inkplate.rtc.setDate(3, 12, 11, 2025);

    // Background color
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create label for displaying date/time
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Draw initial time
    updateTimeLabel(label);
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    lastRefresh = millis();
}

void loop()
{
    // Refresh display every minute
    if (millis() - lastRefresh > REFRESH_DELAY)
    {
        updateTimeLabel(lv_obj_get_child(lv_screen_active(), 0));
        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();
        lastRefresh = millis();
    }
}

// Update LVGL label with current time and date
void updateTimeLabel(lv_obj_t *label)
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
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Recenter label
}