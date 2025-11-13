/**
 **************************************************
 *
 * @file        EEPROMUsage.ino
 * @brief       Example showing  how to Calibrate the RTC to be more precise and accurate.
 *              If you have any issues with the time precision, in this way you can change the internal capacitor value, 
 *              and set the clock offset. Please follow the instructions below carefully.
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>

Inkplate inkplate; // LVGL-enabled Inkplate instance

#define REFRESH_DELAY 1000 // Read RTC every second
unsigned long time1 = 0;

// RTC start values
uint8_t hours = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

// Pointers to LVGL labels
lv_obj_t *mainLabel = nullptr;

// update LVGL label and refresh display
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

    // Initial message
    setLabel("RTC calibration\n\n"
             "Open Serial Monitor\n"
             "at 115200 baud.");

    // Configure button input
    pinMode(GPIO_NUM_36, INPUT);

    // Set internal RTC capacitor (12.5 pF recommended)
    inkplate.rtc.setInternalCapacitor(RTC_12_5PF);

    // Set RTC frequency offset
    inkplate.rtc.setClockOffset(1, -63);

    Serial.println("Press the wake-up button to start RTC");

    setLabel("RTC calibration\n\n"
             "Press Wake button to start.");

    // Wait for button press
    while (digitalRead(GPIO_NUM_36) == HIGH)
    {
        delay(10);
    }

    // Initialize RTC time
    inkplate.rtc.setTime(hours, minutes, seconds);

    setLabel("RTC started.\n"
             "Tracking time...\n"
             "See Serial Monitor at 115200 baud.");
}

void loop()
{
    // Print new time every second
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