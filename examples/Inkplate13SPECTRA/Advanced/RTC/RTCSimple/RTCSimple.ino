/**
 **************************************************
 *
 * @file        RTCSimple.ino
 * @brief       Example showing the functionalities of the onboard RTC by setting its
 *              date and time and refreshing the screen every minute
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

#define REFRESH_DELAY 60000 // 1 minute refresh interval (ms)
unsigned long lastRefresh = 0;

void updateTimeLabel(lv_obj_t *label);

void setup()
{
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);
    inkplate.rtc.reset();

    // Set initial time: 14:30:00
    inkplate.rtc.setTime(14, 30, 0);
    // Wednesday, 12.11.2025
    inkplate.rtc.setDate(3, 12, 11, 2025);

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    updateTimeLabel(label);
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    lastRefresh = millis();
}

void loop()
{
    if (millis() - lastRefresh > REFRESH_DELAY)
    {
        updateTimeLabel(lv_obj_get_child(lv_screen_active(), 0));
        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();
        lastRefresh = millis();
    }
}

void updateTimeLabel(lv_obj_t *label)
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

    lv_label_set_text(label, timeText);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
