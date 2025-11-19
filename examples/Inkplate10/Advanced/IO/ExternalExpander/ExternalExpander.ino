/**
 **************************************************
 *
 * @file        ExternalExpander.ino
 * @brief       Example showing how to set up the external IO expander to blink an LED
 *              on Inkplate 10 using the Inkplate-LVGL library.
 *
 * For info on how to quickly get started with Inkplate 10 visit:
 * https://soldered.com/documentation/inkplate/10/overview/
 * 
 * Connect a 330 Ohm resistor and an LED to pin P1-7 (GPB7) on the "IO Expander 2" header:
 *  - P1-7 (GPB7) -> 330 Ohm resistor -> LED anode (+)
 *  - LED cathode (-) -> GND
 *
 * On this expander all pins are free to use by default. Nothing is connected internally.
 *
 * Pin mapping on the external IO expander:
 *  - GPA0 = 0, GPA1 = 1, ..., GPA7 = 7
 *  - GPB0 = 8, GPB1 = 9, ..., GPB7 = 15
 *  - P0-0 = GPA0, P0-1 = GPA1, ..., P0-7 = GPA7
 *  - P1-0 = GPB0, P1-1 = GPB1, ..., P1-7 = GPB7
 *
 * In this example we use P1-7 = GPB7 = pin 15.
 *
 * LVGL is initialized in PARTIAL render mode. The display backend is configured
 * for L8 (8-bit grayscale) in the Inkplate-LVGL configuration.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Board selection safety check (supports Inkplate 10 and Inkplate 10 V2)
#if !defined(ARDUINO_INKPLATE10) && !defined(ARDUINO_INKPLATE10V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate10 or Soldered Inkplate10 in the boards menu."
#endif

// Include the Inkplate LVGL library
#include "Inkplate-LVGL.h"

// We are going to use pin P1-7 (or GPB7 on older Inkplates).
// P1-7 = GPB7 = 15 in the external IO expander index.
#define LED_PIN 15

// Create an instance of the Inkplate object
Inkplate display;

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("External IO Expander LED blink on Inkplate 10 (LVGL / L8 backend)");

    // Init Inkplate library and LVGL in PARTIAL render mode
    display.begin(LV_DISP_RENDER_MODE_PARTIAL);

    // Set LED pin (P1-7 / GPB7 / 15) as output on the external IO expander
    display.externalIO.pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    // Turn LED off (set output LOW)
    display.externalIO.digitalWrite(LED_PIN, LOW);
    delay(1000); // Wait for one second

    // Turn LED on (set output HIGH)
    display.externalIO.digitalWrite(LED_PIN, HIGH);
    delay(1000); // Wait for one second
}
