/**
 **************************************************
 *
 * @file        DeepSleepButton.ino
 * @brief       Example showing how to wake up the Inkplate 7SPECTRA from deep sleep
 *              by pressing the wake button (GPIO 18), or automatically after 30 seconds.
 *
 * For info on how to quickly get started with Inkplate 7SPECTRA visit
 * https://docs.soldered.com/inkplate/7spectra/quick-start-guide/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_ESP32S3_DEV)
#error "Wrong board selection for this example, please select ESP32S3 Dev Module in the boards menu."
#endif

#include <Inkplate-LVGL.h>

#define uS_TO_S_FACTOR 1000000 // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP  30      // Sleep duration in seconds

RTC_DATA_ATTR int bootCount = 0; // Persistent boot counter across deep sleep cycles

Inkplate inkplate;

void setup()
{
    Serial.begin(115200);

    // Initialize display in FULL mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);

    // Increment persistent boot counter
    bootCount++;

    // Determine wake-up reason
    esp_sleep_wakeup_cause_t wakeup_reason = esp_sleep_get_wakeup_cause();
    const char *reasonText = nullptr;
    switch (wakeup_reason)
    {
        case ESP_SLEEP_WAKEUP_EXT0:
            reasonText = "Wakeup: Button";
            break;
        case ESP_SLEEP_WAKEUP_TIMER:
            reasonText = "Wakeup: Timer";
            break;
        default:
            reasonText = "Wakeup: Power on / Reset";
            break;
    }

    // Prepare text for the LVGL label
    char labelText[128];
    snprintf(labelText, sizeof(labelText), "Boot count: %d\n%s", bootCount, reasonText);

    // Set screen background to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create LVGL label in the center
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, labelText);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Render and display
    lv_timer_handler();
    inkplate.display();

    Serial.printf("Boot count: %d | %s\n", bootCount, reasonText);

    // Enable timer wake-up after TIME_TO_SLEEP seconds
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    // Enable wake button on GPIO 18 (active low)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_18, 0);

    // Enter deep sleep
    esp_deep_sleep_start();
}

void loop()
{
    // Never reached
}
