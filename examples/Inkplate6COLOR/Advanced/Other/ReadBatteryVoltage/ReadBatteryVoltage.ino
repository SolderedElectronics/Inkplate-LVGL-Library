/**
 **************************************************
 *
 * @file        ImageFromBuffer.ino
 * @brief       Example showing how to read the battery voltage and display it on the screen
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/


#include <Inkplate-LVGL.h>

Inkplate inkplate; // Create an instance of the Inkplate object

// Helper function to update LVGL label and refresh display
void updateLabel(lv_obj_t *label, const char *text)
{
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}

void setup()
{
    // Initialize Inkplate in FULL mode (required for LVGL)
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create LVGL label for voltage display
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(0x0000AA), LV_PART_MAIN); // blue-ish
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

void loop()
{
    float voltage = inkplate.readBattery(); // Read battery voltage

    // Format voltage text
    char txt[32];
    snprintf(txt, sizeof(txt), "%.2f V", voltage);

    // Update existing LVGL label (child #0)
    lv_obj_t *label = lv_obj_get_child(lv_screen_active(), 0);
    updateLabel(label, txt);

    delay(60000); // Refresh every 60 seconds
}
