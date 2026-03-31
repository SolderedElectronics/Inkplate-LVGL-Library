/**
 **************************************************
 *
 * @file        RTCCalibration.ino 
 * @brief       Example showing how to calibrate the RTC on Inkplate 6FLICK.
 *              Uses LVGL in grayscale (L8) mode.
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit:
 * https://soldered.com/documentation/inkplate/6flick/overview/
 *
 ***************************************************/

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT); // LVGL-enabled Inkplate instance

#define REFRESH_DELAY 1000 // Read RTC every second
unsigned long time1 = 0;

// RTC start values
uint8_t hours   = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

// Pointers to LVGL labels
lv_obj_t *mainLabel = nullptr;

void setLabel(const char *text)
{
    lv_label_set_text(mainLabel, text);
    lv_obj_align(mainLabel, LV_ALIGN_CENTER, 0, 0);

    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}

void setup()
{
    Serial.begin(115200);

    // Initialize Inkplate in LVGL FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(),
                              lv_color_hex(0xFFFFFF),
                              LV_PART_MAIN);

    // Create primary LVGL label
    mainLabel = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(mainLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(mainLabel, lv_color_hex(0x000000), LV_PART_MAIN);

    setLabel("RTC calibration\n\nOpen Serial Monitor\n115200 baud.");

    // Wake button on Inkplate 6FLICK = GPIO 36
    pinMode(GPIO_NUM_36, INPUT);

    inkplate.rtc.setInternalCapacitor(RTC_12_5PF);
    inkplate.rtc.setClockOffset(1, -63);

    Serial.println("Press Wake button to start RTC");

    setLabel("RTC calibration\n\nPress Wake button to start.");

    while (digitalRead(GPIO_NUM_36) == HIGH)
        delay(10);

    inkplate.rtc.setTime(hours, minutes, seconds);

    setLabel("RTC started.\nTracking time...\nCheck Serial Monitor.");
}

void loop()
{
    if (millis() - time1 > REFRESH_DELAY)
    {
        inkplate.rtc.getRtcData();

        seconds = inkplate.rtc.getSecond();
        minutes = inkplate.rtc.getMinute();
        hours   = inkplate.rtc.getHour();

        Serial.printf("%02d:%02d:%02d\n", hours, minutes, seconds);

        time1 = millis();
    }
}