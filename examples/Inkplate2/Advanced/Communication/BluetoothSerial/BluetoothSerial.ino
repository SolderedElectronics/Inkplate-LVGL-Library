/**
 **************************************************
 *
 * @file        BluetoothSerial.ino
 * @brief       Example showing how to use Bluetooth Serial on Inkplate 2
 *              with an LVGL-based UI that safely renders incoming text.
 *
 * HOW TO USE
 *  - Upload this example to Inkplate 2.
 *  - Pair your phone with the device in Bluetooth settings (device name: "Inkplate 2").
 *  - Open a Serial Bluetooth Terminal app (or similar) and connect to "Inkplate 2".
 *  - Anything you type in the app will be shown on the serial monitor.
 *  - Anything you type in the Arduino Serial Monitor will be sent to the phone.
 *
 *
 * For info on how to quickly get started with Inkplate 2 visit
 * https://soldered.com/documentation/inkplate/2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 **************************************************
 */


#include <Inkplate-LVGL.h>
#include "BluetoothSerial.h"

Inkplate inkplate;        // Inkplate LVGL display instance
BluetoothSerial SerialBT; // Bluetooth Serial object

const char *btDeviceName = "Inkplate 2"; // Bluetooth device name

// Forward declaration
void setLabelText(lv_obj_t *label, const char *text);

void setup()
{
    Serial.begin(115200);

    // Initialize Inkplate in FULL render mode (for LVGL)
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // Set white background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    // Create centered LVGL label
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_8, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    // Initial startup message
    Serial.println("Initializing Bluetooth...");

    // Attempt to start Bluetooth
    if (!SerialBT.begin(btDeviceName))
    {
        setLabelText(label, "Error initializing Bluetooth!");
    }
    else
    {
        // Success message
        setLabelText(label,
                     "Bluetooth Serial\n\n"
                     "Device: Inkplate 2\n\n"
                     "Pair your phone and connect\n"
                     "using a Serial Bluetooth app.\n\n"
                     "Baud: 115200");
    }

    // Render the LVGL content
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}

void loop()
{
    // Forward data from USB Serial → Bluetooth
    if (Serial.available())
    {
        while (Serial.available())
        {
            SerialBT.write(Serial.read());
        }
    }

    // Forward data from Bluetooth → USB Serial
    if (SerialBT.available())
    {
        while (SerialBT.available())
        {
            Serial.write(SerialBT.read());
        }
    }

    delay(20);
}


// update LVGL label text and refresh display
void setLabelText(lv_obj_t *label, const char *text)
{
    lv_label_set_text(label, text);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);

    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}