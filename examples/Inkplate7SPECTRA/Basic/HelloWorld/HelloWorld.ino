/**
 **************************************************
 *
 * @file        HelloWorld.ino
 * @brief       Example showing how to display a simple LVGL label on the Inkplate 7SPECTRA
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

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display
Inkplate inkplate;

void setup()
{
    /* Initialize the display as well as LVGL itself in FULL render mode.
       Dithering is supported in FULL render mode, which allows
       the display to show a wide range of colors. */
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);

    // Change the active screen's background color to white
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create a black label, set its text and font and align it to the center
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Hello world!");
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_48, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Tick the LVGL timer by 50

    // Handle the new label and write it into the framebuffer
    lv_timer_handler();

    // Display the created label onto the screen
    inkplate.display();
}

void loop()
{
    // Stays empty, label is only rendered once to the screen
}
