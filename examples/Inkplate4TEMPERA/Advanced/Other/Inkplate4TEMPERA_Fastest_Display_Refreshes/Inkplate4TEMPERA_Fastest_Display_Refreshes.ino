/**
 **************************************************
 * @file        Inkplate4TEMPERA_Faster_Display_Refreshes_LVGL.ino
 * @brief       Faster partial refresh demo using LVGL on Inkplate 4 TEMPERA.
 *
 * @details     Demonstrates how to speed up consecutive partial updates by
 *              keeping the e-paper panel powered on during repeated refreshes.
 *              The example scrolls text across the screen using LVGL and
 *              partial updates while the panel is powered on via einkOn().
 *
 *              Inkplate 4 TEMPERA resolution is 600x600.
 *
 * @author      Soldered
 * @date        2026
 **************************************************/

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>

// 1-bit mode supports partial update
Inkplate inkplate(INKPLATE_1BIT);

// Text to scroll
const char *text = "Inkplate partial update scrolling demo";

// Scroll settings
int textY = 260;
int textSpeed = 8;

// LVGL object
static lv_obj_t *lbl_text = nullptr;

// Helper function to force LVGL redraw
static void refreshLVGL()
{
    lv_timer_handler();
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("Starting Inkplate 4 TEMPERA LVGL faster refresh example...");

    // Initialize Inkplate + LVGL
    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create scrolling label
    lbl_text = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(lbl_text, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl_text, &lv_font_montserrat_28, 0);
    lv_label_set_long_mode(lbl_text, LV_LABEL_LONG_CLIP);
    lv_label_set_text(lbl_text, text);

    // Set width to match Inkplate 4 TEMPERA usable width
    lv_obj_set_width(lbl_text, 600);

    // Initial off-screen position
    lv_obj_set_pos(lbl_text, -600, textY);

    // Initial full render
    refreshLVGL();
    inkplate.display();
}

void loop()
{
    // Let LVGL calculate real label size
    refreshLVGL();

    int labelWidth = lv_obj_get_content_width(lbl_text);
    if (labelWidth <= 0)
    {
        labelWidth = lv_obj_get_width(lbl_text);
    }

    // Start completely off-screen on the left
    int x = -labelWidth;

    // Keep e-paper power on for faster repeated partial refreshes
    inkplate.einkOn();

    while (x < 600)
    {
        // Move label
        lv_obj_set_pos(lbl_text, x, textY);
        lv_obj_invalidate(lbl_text);

        // Update LVGL drawing
        refreshLVGL();

        // Partial refresh while panel power stays on
        inkplate.partialUpdate(0, 1);

        x += textSpeed;
        delay(80);
    }

    // Turn off e-paper power after animation
    inkplate.einkOff();

    delay(5000);
}