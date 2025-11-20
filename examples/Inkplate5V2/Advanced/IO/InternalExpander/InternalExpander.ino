/**
 **************************************************
 *
 * @file        InternalExpander.ino
 * @brief       Example showing how to use the *internal* IO expander on Inkplate 5 V2
 *              to blink an LED on pin P1-7.
 *
 * For info on how to quickly get started with Inkplate 5 V2 visit
 * https://soldered.com/documentation/inkplate/5/overview/
 * 
 * For this example you will need:
 *  - Inkplate 5 V2
 *  - USB-C cable
 *  - 330 Ohm resistor
 *  - LED diode
 *
 * Hardware connection (INTERNAL IO EXPANDER, header at the top on the backside):
 *  - Connect 330 Ohm resistor to P1-7
 *  - Other side of resistor -> LED anode (+)
 *  - LED cathode (-) -> GND
 *
 * This example shows how you can manipulate I/Os of the *internal* IO expander.
 *
 * You can only freely use Port B pins P1-1 -> P1-7 on the internal expander.
 * Port 0 (Port A, P0-0 -> P0-7) is used for the epaper panel and TPS65186 PMIC.
 *
 * P1-0 is used for ESP32 GPIO0 so you can't use it.
 * P1-1 is used for enabling battery reading (if Batt solder bridge is bridged between second and third pad).
 * P1-2 is used for turning on/off the MOSFET for the SD card (if the solder bridge is bridged between second and third pad).
 *
 * DANGER:
 *  - DO NOT USE P0-0 -> P0-7 and P1-0 on the internal expander.
 *  - In code those are pins 0–8.
 *  - You should only use pins 9–15.
 *  - Using the wrong pins may permanently damage the screen.
 *
 * Pin mapping on the IO expander:
 *  - P0-0 = 0, P0-1 = 1, ..., P0-7 = 7
 *  - P1-0 = 8, P1-1 = 9, ..., P1-7 = 15
 *
 * In this example we use P1-7 = pin 15 on the *internal* IO expander.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE5V2
#error "Wrong board selection for this example, please select Soldered Inkplate5 V2 in the boards menu."
#endif

// Include the Inkplate LVGL Library
#include "Inkplate-LVGL.h"

// We are going to use pin P1-7.
// Remember! P0-0 = 0, P0-1 = 1, ..., P0-7 = 7, P1-0 = 8, P1-1 = 9, ..., P1-7 = 15.
#define LED_PIN 15

// Create an instance of the Inkplate object
Inkplate inkplate(INKPLATE_1BIT);

void setup()
{
    // Init Inkplate library and LVGL in PARTIAL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);

    // Set P1-7 as output on the *internal* IO expander.
    // Only pins 9–15 are safe to use on the internal expander.
    inkplate.internalIO.pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    // Set output to LOW (LED does not light up)
    inkplate.internalIO.digitalWrite(LED_PIN, LOW);
    delay(1000); // Wait for one second

    // Set output to HIGH (LED lights up)
    inkplate.internalIO.digitalWrite(LED_PIN, HIGH);
    delay(1000); // Wait for one second
}
