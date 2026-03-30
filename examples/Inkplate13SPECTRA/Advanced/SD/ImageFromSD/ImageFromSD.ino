/**
 **************************************************
 *
 * @file        ImageFromSD.ino
 * @brief       Example showing how to read jpg files from an SD card and display them on
 *              Inkplate 13 SPECTRA using LVGL. Image files (picture1.jpg, picture2.jpg) can
 *              be found in the folder of this example sketch — copy them to the root of your
 *              SD card. Format your SD card using the standard FAT format.
 *
 *              The example displays picture1.jpg for 5 seconds, then switches to picture2.jpg.
 *              To use different images, change the filenames in the lv_image_set_src() calls below.
 *
 * For info on how to quickly get started with Inkplate 13 SPECTRA visit https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

#define USE_COLOR_IMAGE
#include <Inkplate-LVGL.h>

// JPEG decoding requires significant stack space — increase the loop task stack to avoid overflow
SET_LOOP_TASK_STACK_SIZE(65 * 1024);

// Create an instance of the Inkplate object
Inkplate inkplate;

// Helper: display a single jpg from the SD card and trigger a full screen refresh
static void showImage(const char *path)
{
    lv_obj_clean(lv_scr_act());
    lv_obj_set_style_bg_color(lv_scr_act(), lv_color_black(), 0);
    lv_obj_set_style_bg_opa(lv_scr_act(), LV_OPA_COVER, 0);

    lv_obj_t *img = lv_image_create(lv_scr_act());
    lv_image_set_src(img, path);  // 'S:/' is the LVGL drive letter for the SD card
    lv_obj_center(img);

    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}

void setup()
{
    Serial.begin(115200);
    delay(1000);

    // Initialize display and LVGL in full render mode with dithering
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);
    Serial.println("Display initialized");

    // Initialize SD card
    if (!inkplate.sdCardInit())
    {
        Serial.println("SD Card init failed!");
        return;
    }
    Serial.println("SD Card initialized");

    // Register LVGL filesystem driver (mounts SD as drive 'S')
    lv_fs_init_sd();

    // --- Display picture1.jpg ---
    SdFile f1;
    if (f1.open("picture1.jpg", O_READ))
    {
        Serial.printf("picture1.jpg found (%lu bytes)\n", f1.fileSize());
        f1.close();

        Serial.println("Showing picture1.jpg...");
        showImage("S:/picture1.jpg");
        delay(5000);
    }
    else
    {
        Serial.println("picture1.jpg NOT found on SD card!");
    }

    // --- Display picture2.jpg ---
    SdFile f2;
    if (f2.open("picture2.jpg", O_READ))
    {
        Serial.printf("picture2.jpg found (%lu bytes)\n", f2.fileSize());
        f2.close();

        Serial.println("Showing picture2.jpg...");
        showImage("S:/picture2.jpg");
        delay(5000);
    }
    else
    {
        Serial.println("picture2.jpg NOT found on SD card!");
    }

    // Turn off the SD card to save power
    inkplate.sdCardSleep();
    Serial.println("Done.");
}

void loop()
{
    // Nothing — e-paper retains the last image
}
