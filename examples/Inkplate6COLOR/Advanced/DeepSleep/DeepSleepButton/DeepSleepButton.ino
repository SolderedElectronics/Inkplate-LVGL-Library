/**
 **************************************************
 *
 * @file        DeepSleepButton.ino
 * @brief       The following example shows how to use ESP interrupts to wake up the MCU from deepsleep when wake up button
 *              is pressed. Also, wake up on timer after 30 seconds of deep sleep if the button is not pressed.
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>

// Conversion factor and sleep duration
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 30

// Persistent boot counter across deep sleep cycles
RTC_DATA_ATTR int bootCount = 0;

// Create Inkplate LVGL display object
Inkplate inkplate;

void setup()
{
    Serial.begin(115200);

    // Initialize display (FULL mode for LVGL)
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

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
            reasonText = "Wakeup: Unknown";
            break;
    }

    // Prepare text for the LVGL label
    char labelText[128];
    snprintf(labelText, sizeof(labelText),
             "Boot count: %d\n%s", bootCount, reasonText);

    // Set screen background to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create LVGL label in the center
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, labelText);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_26, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Render and display
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    Serial.printf("Boot count: %d | %s\n", bootCount, reasonText);

    // Enable timer wake-up after TIME_TO_SLEEP seconds
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    // Enable button wake-up on GPIO 36 (active low)
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, 0);

    // Enter deep sleep
    esp_deep_sleep_start();
}

void loop()
{
    // Never reached
}