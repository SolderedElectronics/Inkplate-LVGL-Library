/**
 **************************************************
 *
 * @file        ReadBatteryVoltage.ino
 * @brief       Example showing how to read Inkplate 10 battery voltage
 *              and display it using LVGL.
 *
 * For info on Inkplate 10:
 * https://soldered.com/documentation/inkplate/10/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************
 */

#if !defined(ARDUINO_INKPLATE10) && !defined(ARDUINO_INKPLATE10V2)
#error "Wrong board selected. Please choose e-radionica Inkplate10 or Soldered Inkplate10."
#endif

#include <Inkplate-LVGL.h>

// Inkplate in 1-bit mode (black & white)
Inkplate inkplate(INKPLATE_1BIT);

// LVGL label for voltage
static lv_obj_t *lbl_voltage;

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("Starting ReadBattery LVGL example on Inkplate 10...");

    // Init LVGL + Inkplate in partial render mode
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(),
                              lv_color_hex(0xFFFFFF),
                              LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create voltage label (big font, centered)
    lbl_voltage = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(lbl_voltage, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl_voltage, &lv_font_montserrat_48, 0);
    lv_obj_center(lbl_voltage);

    // First reading immediately
    float voltage = inkplate.readBattery();
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f V", voltage);
    lv_label_set_text(lbl_voltage, buf);

    lv_tick_inc(20);
    lv_timer_handler();
    inkplate.display();   // First refresh is FULL
}

void loop()
{
    float voltage = inkplate.readBattery();

    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f V", voltage);
    lv_label_set_text(lbl_voltage, buf);
    lv_obj_center(lbl_voltage);

    // Update LVGL internal buffer
    lv_tick_inc(20);
    lv_timer_handler();

    // Fast partial update
    inkplate.partialUpdate();

    delay(10000); // Every 10 s
}
