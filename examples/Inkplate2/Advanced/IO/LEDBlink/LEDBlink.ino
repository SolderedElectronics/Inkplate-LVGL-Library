/**
 **************************************************
 *
 * @file        LEDBlink.ino
 * @brief       Example showing how to blink an LED on IO15 pin of the Inkplate 2 board
 *
 * For info on how to quickly get started with Inkplate 2 visit https://soldered.com/documentation/inkplate/2/overview/
 * 
 * Connect an LED alongside a 330ohm resistor to pin IO15
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include "Inkplate-LVGL.h" //Include Inkplate library to the sketch


Inkplate inkplate; // Create an instance of the Inkplate object

#define LED_PIN 15 // Change this if you want to use another pin

void setup()
{
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL); // Init Inkplate library and LVGL in PARTIAL render mode

    // Set the mode of the LED_PIN
    pinMode(LED_PIN, OUTPUT);
}

void loop()
{
    // Set the LED pin to LOW
    digitalWrite(LED_PIN, LOW);
    delay(1000);                           // Wait for one second
    // Set the LED pin to HIGH
    digitalWrite(LED_PIN, HIGH);
    delay(1000);                           // Wait for one second
}