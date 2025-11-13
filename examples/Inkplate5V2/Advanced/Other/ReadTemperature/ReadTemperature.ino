/**
 **************************************************
 *
 * @file        ReadTemperature.ino
 * @brief       Example showing how to read the onboard temperature
 *              sensor on Inkplate 5V2 and display the value using LVGL.
 *
 * For info on Inkplate 5V2:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * NOTE:
 *   Inkplate 5V2 ima isti API za temperaturu kao i ostale Inkplate modele,
 *   tako da se samo mijenja board selection i rezolucija se automatski skalira kroz LVGL.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************
 */

// Optional board check (možeš i ukloniti ako želiš)
#if !defined(ARDUINO_INKPLATE5V2)
#error "Wrong board selected. Please choose Soldered Inkplate5 V2."
#endif

#include <Inkplate-LVGL.h>

// Inkplate in 1-bit mode (black & white)
Inkplate inkplate(INKPLATE_1BIT);

// LVGL label for temperature
static lv_obj_t *lbl_temp;

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("Starting ReadTemperature LVGL example (Inkplate 5V2)...");

    // Init LVGL + Inkplate in partial render mode (faster)
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // White background
    lv_obj_set_style_bg_color(
        lv_screen_active(),
        lv_color_hex(0xFFFFFF),
        LV_PART_MAIN
    );
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create temperature label (large clean font)
    lbl_temp = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(lbl_temp, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl_temp, &lv_font_montserrat_48, 0);
    lv_obj_center(lbl_temp);

    // First reading
    int t = inkplate.readTemperature();
    char buf[32];
    snprintf(buf, sizeof(buf), "%d C", t);
    lv_label_set_text(lbl_temp, buf);

    // Update LVGL + first FULL refresh
    lv_tick_inc(20);
    lv_timer_handler();
    inkplate.display();
}

void loop()
{
    int temperature = inkplate.readTemperature();

    char buf[32];
    snprintf(buf, sizeof(buf), "%d C", temperature);
    lv_label_set_text(lbl_temp, buf);
    lv_obj_center(lbl_temp);

    // Update LVGL state
    lv_tick_inc(20);
    lv_timer_handler();

    // Partial refresh (fast)
    inkplate.partialUpdate();

    delay(10000); // Update every 10 seconds
}
