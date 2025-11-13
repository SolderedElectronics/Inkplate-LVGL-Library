/**
 **************************************************
 *
 * @file        ImageFromBuffer.ino
 * @brief       Example showing how to display a 1-bit black & white
 *              image stored in a buffer using LVGL on Inkplate 6.
 *
 *              The image array data was generated using e.g.:
 *              https://notisrac.github.io/FileToCArray/
 *
 *              Recommended generator options:
 *                - Code format: Hex(0x00)
 *                - Palette / format: 1-bit monochrome (black & white)
 *                - Multi line
 *                - Separate bytes of pixels
 *                - Other options can stay default
 *
 *              The generated header file (cat.h) must provide:
 *                - const uint16_t cat_w;   // image width in pixels
 *                - const uint16_t cat_h;   // image height in pixels
 *                - const uint8_t  cat[];   // 1-bit bitmap, size = cat_w * cat_h / 8
 *
 *              This example treats the buffer as LVGL A1 format
 *              (1-bit alpha mask) and draws it as black on white.
 *
 * For info on how to quickly get started with Inkplate 6 visit:
 * https://soldered.com/documentation/inkplate/6/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************
 */

#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include <Inkplate-LVGL.h>

// Include the image data (defines cat_w, cat_h and cat[])
#include "cat.h"

// Inkplate in 1-bit (black & white) mode
Inkplate inkplate(INKPLATE_1BIT);

// LVGL image descriptor for 1-bit alpha bitmap
static const lv_image_dsc_t my_image = {
    .header = {
        .cf = LV_COLOR_FORMAT_A1,         // 1-bit alpha mask
        .w  = (uint16_t)cat_w,
        .h  = (uint16_t)cat_h,
    },
    .data_size = sizeof(cat),            // exact size of the array
    .data      = cat,
};

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("ImageFromBuffer (1-bit) example start");

    // Initialize Inkplate + LVGL in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create LVGL image object
    lv_obj_t *img = lv_image_create(lv_screen_active());
    lv_image_set_src(img, &my_image);

    // Draw mask as black on white
    lv_obj_set_style_img_recolor_opa(img, LV_OPA_COVER, 0);
    lv_obj_set_style_img_recolor(img, lv_color_hex(0x000000), 0);

    // Center the image on the screen
    lv_obj_center(img);

    // Let LVGL render into its buffer
    lv_tick_inc(20);
    lv_timer_handler();

    // Push buffer to e-paper
    inkplate.display();

    Serial.println("Image drawn.");
}

void loop()
{
    // Static image, nothing to do
}
