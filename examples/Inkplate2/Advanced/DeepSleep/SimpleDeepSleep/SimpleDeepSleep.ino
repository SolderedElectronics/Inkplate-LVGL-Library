/**
 **************************************************
 *
 * @file        SimpleDeepSleep.ino
 * @brief       Example showing how to put the Inkplate device into deep sleep, wake it up every 30 seconds and
 *              display the wakeup count onto the screen     
 *
 * For info on how to quickly get started with Inkplate 2 visit https://soldered.com/documentation/inkplate/2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>
#include "driver/rtc_io.h"

#define uS_TO_S_FACTOR 1000000  // Conversion factor for microseconds to seconds
#define TIME_TO_SLEEP  30       // Sleep duration in seconds

RTC_DATA_ATTR int wakeCount = 0; // Retained variable across deep sleep cycles

Inkplate inkplate; // Create Inkplate LVGL instance

void setup()
{
    Serial.begin(115200);

    // Initialize Inkplate in FULL render mode (needed for LVGL and color)
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // Increment persistent counter
    wakeCount++;

    // Clear screen background to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create label text buffer
    char labelText[64];
    snprintf(labelText, sizeof(labelText), "Wake count: %d", wakeCount);

    // Create LVGL label and style it
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, labelText);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_10, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Render label and update display
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    Serial.printf("Device wake count: %d\n", wakeCount);

    // Configure deep sleep wake-up timer
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    // Go to deep sleep
    esp_deep_sleep_start();
}

void loop()
{
    // Never reached; all logic in setup()
}