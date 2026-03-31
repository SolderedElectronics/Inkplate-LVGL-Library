/**
 **************************************************
 *
 * @file        RTCCalibration.ino
 * @brief       Example showing how to calibrate the RTC to be more precise and accurate.
 *              If you have any issues with time precision, you can change the internal
 *              capacitor value and set the clock offset. Please follow the instructions
 *              below carefully.
 *
 * For info on how to quickly get started with Inkplate 13SPECTRA visit https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

#define USE_COLOR_IMAGE
#include <Inkplate-LVGL.h>

Inkplate inkplate;

#define REFRESH_DELAY 1000 // Read RTC every second
unsigned long time1 = 0;

// RTC start values
uint8_t hours   = 0;
uint8_t minutes = 0;
uint8_t seconds = 0;

// Pointer to LVGL label
lv_obj_t *mainLabel = nullptr;

// Update LVGL label and refresh display
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
    inkplate.enableDithering(true);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create primary LVGL label
    mainLabel = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(mainLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(mainLabel, lv_color_hex(0x000000), LV_PART_MAIN);

    // Initial message
    setLabel("RTC calibration\n\n"
             "Open Serial Monitor\n"
             "at 115200 baud.");

    // Configure wake button input (GPIO 18 on Inkplate 13SPECTRA)
    pinMode(GPIO_NUM_18, INPUT);

    // Set internal RTC capacitor (12.5 pF recommended)
    inkplate.rtc.setInternalCapacitor(RTC_12_5PF);

    // Set RTC frequency offset
    // Mode 1 (course): offset every 4 minutes, 4.069 ppm per LSB, range -64 to +63
    // Mode 0 (normal): offset every 2 hours,  4.34  ppm per LSB, range -64 to +63
    inkplate.rtc.setClockOffset(1, -63);

    Serial.println("Press the wake-up button to start RTC");

    setLabel("RTC calibration\n\n"
             "Press Wake button to start.");

    // Wait for button press
    while (digitalRead(GPIO_NUM_18) == HIGH)
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
