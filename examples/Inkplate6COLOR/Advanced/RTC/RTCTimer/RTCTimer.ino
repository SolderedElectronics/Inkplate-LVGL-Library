/**
 **************************************************
 *
 * @file        RTCTimer.ino
 * @brief       Example showing how to set the timer and when it triggers, display "Timer!" along with the current date and time 
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Refresh interval (1 minute)
#define REFRESH_DELAY 60000

#include <Inkplate-LVGL.h>

Inkplate inkplate; // Inkplate LVGL instance

// Timer setup (seconds)
int countdown_time = 60;

unsigned long lastRefresh = 0;

// Forward declarations
void updateTimeLabel(lv_obj_t *label);

void setup()
{
    // Initialize Inkplate in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // Reset the time the RTC stores
    inkplate.rtc.reset();

    // Configure RTC INT pin
    pinMode(39, INPUT_PULLUP);

    // Set initial time and date
    // 14:30:00
    inkplate.rtc.setTime(14, 30, 0);
    // Wednesday, 12.11.2025.
    inkplate.rtc.setDate(3, 12, 11, 2025);

    // Configure RTC timer: 1Hz clock, countdown_time seconds, interrupt enabled
    inkplate.rtc.timerSet(RTC::TIMER_CLOCK_1HZ, countdown_time, true, false);

    // Clear background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create centered LVGL label
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
    // Update every minute
    if (millis() - lastRefresh > REFRESH_DELAY)
    {
        updateTimeLabel(lv_obj_get_child(lv_screen_active(), 0));
        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();
        lastRefresh = millis();
    }
}

// Update label with current time/date and timer event
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

    char labelText[160];
    snprintf(labelText, sizeof(labelText),
             "%02d:%02d:%02d\n%s, %02d/%02d/%04d",
             hour, minute, second, wdayNames[weekday], day, month, year);

    // If the RTC timer event occurred, show a "Timer!" message
    if (inkplate.rtc.checkTimerFlag())
    {
        inkplate.rtc.clearTimerFlag(); // Clear timer flag
        inkplate.rtc.disableTimer();   // Disable timer if one-time only
        strcat(labelText, "\n\nTimer!");
    }

    lv_label_set_text(label, labelText);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}
