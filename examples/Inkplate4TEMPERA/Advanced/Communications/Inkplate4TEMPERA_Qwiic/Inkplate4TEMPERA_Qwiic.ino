/**
 **************************************************
 *
 * @file        Inkplate4TEMPERA_Qwiic_LVGL.ino
 * @brief       LVGL-based I2C (Qwiic) scanner example for Soldered Inkplate 4TEMPERA.
 *
 *              This example scans the I2C bus for connected Qwiic/I2C devices and
 *              displays detected device addresses on the Inkplate 4TEMPERA e-paper
 *              display using LVGL labels. Detected addresses are also printed to
 *              the Serial Monitor.
 *
 *              The scan repeats every 5 seconds.
 *
 * @hardware    Soldered Inkplate 4TEMPERA
 * @library     Inkplate-LVGL
 * @authors     Soldered
 * @date        March 2026
 **************************************************
 */

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Soldered Inkplate 4TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <Wire.h>

// Inkplate in 1-bit mode
Inkplate inkplate(INKPLATE_1BIT);

// UI elements
static lv_obj_t *lbl_title;
static lv_obj_t *lbl_status;
static lv_obj_t *lbl_results;

// Refresh policy
static int partialCount = 0;
static const int FULL_REFRESH_EVERY = 20;

// Helpers
static inline int dispW() { auto *d = lv_display_get_default(); return lv_display_get_horizontal_resolution(d); }
static inline int dispH() { auto *d = lv_display_get_default(); return lv_display_get_vertical_resolution(d); }

static void make_opaque(lv_obj_t *obj)
{
    lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
    lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(obj, 0, 0);
}

static void ui_create()
{
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    lbl_title = lv_label_create(lv_screen_active());
    lv_label_set_text(lbl_title, "Inkplate I2C Scanner");
    lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(lbl_title, 8, 8);
    make_opaque(lbl_title);

    lbl_status = lv_label_create(lv_screen_active());
    lv_label_set_text(lbl_status, "Initializing...");
    lv_obj_set_style_text_color(lbl_status, lv_color_hex(0x000000), 0);
    lv_obj_set_pos(lbl_status, 8, 32);
    make_opaque(lbl_status);

    lbl_results = lv_label_create(lv_screen_active());
    lv_label_set_text(lbl_results, "");
    lv_obj_set_style_text_color(lbl_results, lv_color_hex(0x000000), 0);
    lv_obj_set_width(lbl_results, dispW() - 16);
    lv_obj_set_pos(lbl_results, 8, 56);
    make_opaque(lbl_results);
}

static void scan_i2c_devices()
{
    byte error, address;
    int nDevices = 0;

    char resultBuf[1024];
    resultBuf[0] = '\0';

    lv_label_set_text(lbl_status, "Scanning I2C...");
    lv_label_set_text(lbl_results, "");

    Serial.println("Scanning...");

    for (address = 1; address < 127; address++)
    {
        Wire.beginTransmission(address);
        error = Wire.endTransmission();

        if (error == 0)
        {
            char line[32];
            snprintf(line, sizeof(line), "Found: 0x%02X\n", address);

            if (strlen(resultBuf) + strlen(line) < sizeof(resultBuf) - 1)
            {
                strcat(resultBuf, line);
            }

            Serial.print("I2C device found at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.print(address, HEX);
            Serial.println(" !");

            nDevices++;
        }
        else if (error == 4)
        {
            Serial.print("Unknown error at address 0x");
            if (address < 16)
                Serial.print("0");
            Serial.println(address, HEX);
        }
    }

    if (nDevices == 0)
    {
        Serial.println("No I2C devices found\n");
        lv_label_set_text(lbl_status, "Scan complete");
        lv_label_set_text(lbl_results, "No devices found.");
    }
    else
    {
        Serial.println("Done.\n");
        lv_label_set_text(lbl_status, "Scan complete");
        lv_label_set_text(lbl_results, resultBuf);
    }
}

void setup()
{
    Serial.begin(115200);
    Wire.begin();

    // LVGL + Inkplate in partial render mode
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    ui_create();
    scan_i2c_devices();

    // Initial full refresh
    lv_timer_handler();
    inkplate.display();

    Serial.println("\nSoldered Inkplate I2C Scanner!");
}

void loop()
{
    scan_i2c_devices();

    if (partialCount >= FULL_REFRESH_EVERY)
    {
        lv_timer_handler();
        inkplate.display();   // full refresh
        partialCount = 0;
    }
    else
    {
        lv_timer_handler();
        inkplate.partialUpdate();
        partialCount++;
    }

    delay(5000);
}