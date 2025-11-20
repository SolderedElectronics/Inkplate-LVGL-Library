/**
 **************************************************
 *
 * @file        ReadTemperature.ino
 * @brief       Example showing how to read the onboard temperature
 *              sensor on Inkplate 6FLICK and display the value using LVGL.
 *
 *
 * For info on Inkplate 6FLICK:
 * https://soldered.com/documentation/inkplate/6flick/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ****************************************************/

#include <Inkplate-LVGL.h>

// Inkplate in 1-bit mode (black & white)
Inkplate inkplate(INKPLATE_1BIT);

// LVGL label for temperature
static lv_obj_t *lbl_temp;

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("Starting ReadTemperature LVGL example...");

    // Init LVGL + Inkplate in partial render mode (faster)
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create temperature label
    lbl_temp = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(lbl_temp, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl_temp, &lv_font_montserrat_48, 0);  // Big clean font
    lv_obj_center(lbl_temp);

    // Draw first reading immediately
    int t = inkplate.readTemperature();
    char buf[32];
    snprintf(buf, sizeof(buf), "%d C", t);
    lv_label_set_text(lbl_temp, buf);

    lv_tick_inc(20);
    lv_timer_handler();
    inkplate.display();   // Full refresh on first draw
}

void loop()
{
    int temperature = inkplate.readTemperature();

    char buf[32];
    snprintf(buf, sizeof(buf), "%d C", temperature);
    lv_label_set_text(lbl_temp, buf);
    lv_obj_center(lbl_temp);

    // Let LVGL update internal buffer
    lv_tick_inc(20);
    lv_timer_handler();

    // Partial update for faster refresh
    inkplate.partialUpdate();

    delay(10000); // Update every 10 s
}