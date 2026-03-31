/**
 **************************************************
 *
 * @file        RTCTimer.ino
 * @brief       Example showing how to set the RTC timer and display "Timer!" along
 *              with the current date and time when it triggers.
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

#define REFRESH_DELAY 60000

Inkplate inkplate;

int countdown_time = 60; // Timer countdown in seconds
unsigned long lastRefresh = 0;

void updateTimeLabel(lv_obj_t *label);

void setup()
{
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);
    inkplate.rtc.reset();

    // RTC INT pin on GPIO 18 on Inkplate 13SPECTRA
    pinMode(18, INPUT_PULLUP);

    // Set initial time: 14:30:00
    inkplate.rtc.setTime(14, 30, 0);
    // Wednesday, 12.11.2025
    inkplate.rtc.setDate(3, 12, 11, 2025);

    // Configure RTC timer: 1Hz clock, countdown_time seconds, interrupt enabled
    inkplate.rtc.timerSet(RTC::TIMER_CLOCK_1HZ, countdown_time, true, false);

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

    char labelText[160];
    snprintf(labelText, sizeof(labelText),
             "%02d:%02d:%02d\n%s, %02d/%02d/%04d",
             inkplate.rtc.getHour(), inkplate.rtc.getMinute(), inkplate.rtc.getSecond(),
             wdayNames[inkplate.rtc.getWeekday()],
             inkplate.rtc.getDay(), inkplate.rtc.getMonth(), inkplate.rtc.getYear());

    if (inkplate.rtc.checkTimerFlag())
    {
        inkplate.rtc.clearTimerFlag();
        inkplate.rtc.disableTimer();
        strcat(labelText, "\n\nTimer!");
    }

    lv_label_set_text(label, labelText);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
