/**
 **************************************************
 *
 * @file        ExternalExpander.ino
 * @brief       Example showing how to use the IO expander on Inkplate 13 SPECTRA
 *              to blink an LED connected to pin P1-7.
 *
 * Connect a 330 Ohm resistor to pin P1-7 on the IO Expander header (IO Expander 2)
 * on the backside of the Inkplate. Connect the other side of the resistor to the
 * anode of an LED, and the cathode of the LED to GND.
 *
 * Pin mapping:
 *   P0-0 = IO_PIN_A0 = 0, ..., P0-7 = IO_PIN_A7 = 7
 *   P1-0 = IO_PIN_B0 = 8, ..., P1-7 = IO_PIN_B7 = 15
 *
 * For info on how to quickly get started with Inkplate 13 SPECTRA visit
 * https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

// Use full RGB565 color space for LVGL
#define USE_COLOR_IMAGE

#include <Inkplate-LVGL.h>

// We are going to use pin P1-7 (IO_PIN_B7 = 15)
// GPA0 = 0, GPA1 = 1, ..., GPA7 = 7, GPB0 = 8, GPB1 = 9, ..., GPB7 = 15
#define LED_PIN IO_PIN_B7

Inkplate inkplate;

void setup()
{
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);

    // Set P1-7 to output
    inkplate.internalIO.pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    inkplate.internalIO.digitalWrite(LED_PIN, LOW);  // LED off
    delay(1000);
    inkplate.internalIO.digitalWrite(LED_PIN, HIGH); // LED on
    delay(1000);
}
