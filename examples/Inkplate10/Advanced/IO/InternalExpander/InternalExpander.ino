/**
 **************************************************
 *
 * @file        InternalExpander.ino
 * @brief       Example showing how to use the *internal* IO expander on Inkplate 10
 *              to blink an LED on pin P1-7 (GPB7).
 *
 * For info on how to quickly get started with Inkplate 10 visit:
 * https://soldered.com/documentation/inkplate/10/overview/
 * 
 * For this example you will need:
 *  - Inkplate 10
 *  - Micro USB cable
 *  - 330 Ohm resistor
 *  - LED diode
 *
 * Hardware connection (INTERNAL IO EXPANDER, "IO Expander 1" header at the top):
 *  - Connect 330 Ohm resistor to P1-7 (GPB7)
 *  - Other side of resistor -> LED anode (+)
 *  - LED cathode (-) -> GND
 *
 * This example shows how to manipulate I/Os of the *internal* IO expander.
 *
 * You can only freely use Port B pins P1-1 -> P1-7 (GPB1 -> GPB7) on the internal expander.
 * Port A (P0-0 -> P0-7 / GPA0 -> GPA7) is used for the epaper panel and TPS65186 PMIC.
 * P1-0 (GPB0) is used for ESP32 GPIO0 and must not be used as a general-purpose pin.
 *
 * IMPORTANT (DANGER):
 *  - DO NOT USE P0-0 -> P0-7 (GPA0 -> GPA7) or P1-0 (GPB0) on the internal expander.
 *  - In code those are pins 0–8.
 *  - You should only use pins 9–15 on the internal expander.
 *  - Using the wrong pins may permanently damage the screen.
 *
 * Pin mapping on the IO expander:
 *  - GPA0 = 0, GPA1 = 1, ..., GPA7 = 7
 *  - GPB0 = 8, GPB1 = 9, ..., GPB7 = 15
 *  - P0-0 = GPA0, P0-1 = GPA1, ..., P0-7 = GPA7
 *  - P1-0 = GPB0, P1-1 = GPB1, ..., P1-7 = GPB7
 *
 * In this example we use P1-7 = GPB7 = 15 on the *internal* IO expander.
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

// Include the Inkplate LVGL Library
#include "Inkplate-LVGL.h"

// We are going to use pin P1-7 (or GPB7).
// P1-7 = GPB7 = 15 in the IO expander index.
#define LED_PIN 15

// Create an instance of the Inkplate object
Inkplate inkplate(INKPLATE_1BIT);

void setup()
{
    Serial.begin(115200);
    delay(1000);
    Serial.println("Internal IO Expander LED blink on Inkplate 10 (LVGL / L8 backend)");

    // Init Inkplate library and LVGL in PARTIAL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);

    // Set P1-7 (GPB7) as output on the *internal* IO expander
    // Only pins 9–15 are safe to use on the internal expander.
    inkplate.internalIO.pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    // Set output to LOW (LED off)
    inkplate.internalIO.digitalWrite(LED_PIN, LOW);
    delay(1000); // Wait for one second

    // Set output to HIGH (LED on)
    inkplate.internalIO.digitalWrite(LED_PIN, HIGH);
    delay(1000); // Wait for one second
}
