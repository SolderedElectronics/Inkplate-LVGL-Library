/**
 **************************************************
 *
 * @file        ImageFromBuffer.ino
 * @brief       Example showing an 8-bit grayscale JPG from buffer
 *              on Inkplate 5V2 (1280x720)
 *
 * The image was converted with:
 * https://notisrac.github.io/FileToCArray/
 *
 * Used options:
 *   - Code format: Hex (0x00)
 *   - Palette mod: 8-bit grayscale (1 byte/pixel)
 *   - Multi line
 *   - Separate bytes of pixels
 *   - static + const + unsigned + PROGMEM
 *
 * The resulting header (e.g. cat.h) contains:
 *   - #define CAT_WIDTH  1280   (must match your actual image!)
 *   - #define CAT_HEIGHT 720
 *   - static const unsigned char cat[] PROGMEM = { ... };
 *
 * IMPORTANT:
 *   Your image must be resized to exactly 1280x720 for Inkplate 5V2
 *   or LVGL will not render it correctly.
 *
 ***************************************************/
#include <Inkplate-LVGL.h>

// Image data
#include "cat.h"

// Inkplate object (1-bit mode is fine; dithering handles grayscale)
Inkplate inkplate(INKPLATE_1BIT);

lv_obj_t *screen;

// LVGL image descriptor for 8-bit grayscale (L8)
const lv_image_dsc_t my_image = {
    .header = {
        .cf = LV_COLOR_FORMAT_L8,   // 8-bit grayscale / luminance
        .w  = CAT_WIDTH,
        .h  = CAT_HEIGHT,
    },
    .data_size = CAT_WIDTH * CAT_HEIGHT,   // 1 byte per pixel
    .data      = cat,                      // raw grayscale buffer
};

// LVGL task that ticks the timer every 5 ms
void lvgl_task(void *arg) {
    for (;;) {
        lv_tick_inc(5);
        lv_timer_handler();
        vTaskDelay(pdMS_TO_TICKS(5));
    }
}

void setup() {
    Serial.begin(115200);
    delay(1000);
    Serial.println("Starting LVGL grayscale image example on Inkplate 5V2...");

    // Initialize Inkplate + LVGL in full render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // Enable dithering so grayscale images look smoother on BW panel
    inkplate.enableDithering(1);

    // LVGL task on core 1
    xTaskCreatePinnedToCore(
        lvgl_task,
        "lvgl_tick",
        16000,
        nullptr,
        2,
        nullptr,
        1);

    delay(100);

    // Get the active screen
    screen = lv_scr_act();
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // Create LVGL image object
    lv_obj_t *img = lv_image_create(screen);
    lv_image_set_src(img, &my_image);
    lv_obj_center(img);   // auto-centers regardless of resolution

    // Ensure LVGL renders before sending to Inkplate buffer
    for (int i = 0; i < 5; i++) {
        lv_timer_handler();
        delay(10);
    }

    // Refresh Inkplate screen
    inkplate.display();
    Serial.println("Display updated with 8-bit grayscale (L8) image");
}

void loop() {
    // Nothing needed here
}
