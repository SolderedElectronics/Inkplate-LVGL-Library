/**
 **************************************************
 *
 * @file        ReadBatteryVoltage.ino
 * @brief       Example showing how to read Inkplate 5V2 battery voltage
 *              and display it using LVGL.
 *
 * For info on Inkplate 5V2:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * NOTE:
 *   Inkplate 5V2 has the SAME battery-reading API as Inkplate 6,
 *   so the logic stays identical — only resolution and board define differ.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************
 */

// Optional safety check — you may comment out if not needed
#if !defined(ARDUINO_INKPLATE5V2)
#error "Wrong board selected. Please choose Soldered Inkplate5 V2."
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
    Serial.println("Starting ReadBattery LVGL example (Inkplate 5V2)...");

    // Init LVGL + Inkplate in PARTIAL render mode (fast)
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // White background
    lv_obj_set_style_bg_color(
        lv_screen_active(),
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN
    );
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create voltage label with large font
    lbl_voltage = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(lbl_voltage, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl_voltage, &lv_font_montserrat_48, 0);
    lv_obj_center(lbl_voltage);

    // First battery reading
    float voltage = inkplate.readBattery();
    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f V", voltage);
    lv_label_set_text(lbl_voltage, buf);

    // Render + FULL refresh
    lv_tick_inc(20);
    lv_timer_handler();
    inkplate.display();

    Serial.printf("Initial battery voltage: %.2f V\n", voltage);
}

void loop()
{
    // Read battery voltage again
    float voltage = inkplate.readBattery();

    char buf[32];
    snprintf(buf, sizeof(buf), "%.2f V", voltage);
    lv_label_set_text(lbl_voltage, buf);
    lv_obj_center(lbl_voltage);

    // Update LVGL layout
    lv_tick_inc(20);
    lv_timer_handler();

    // Fast partial refresh
    inkplate.partialUpdate();

    Serial.printf("Battery: %.2f V\n", voltage);

    delay(10000); // Update every 10 seconds
}
