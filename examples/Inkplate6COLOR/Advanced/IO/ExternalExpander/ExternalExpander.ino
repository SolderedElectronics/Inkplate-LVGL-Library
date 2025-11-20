/**
 **************************************************
 *
 * @file        ExternalExpander.ino
 * @brief       Example showing how to set up the external expander to blink an LED on all of the pins
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 * 
 * Connect an LED alongside a 330ohm resistor to one of the external expander pins
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include "Inkplate-LVGL.h" //Include Inkplate library to the sketch


Inkplate inkplate; // Create an instance of the Inkplate object

void setup()
{
    inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL); // Init Inkplate library and LVGL in PARTIAL rende mode
    // Set all of the pins (P0-0 to P1-7) to output
    // Note: P0-0 is defined as 0, up to P1-7 which is defined as 15
    for(int i=0;i<16;i++)
    {
        inkplate.externalIO.pinMode(i, OUTPUT);
    }
}

void loop()
{
    // Set all of the GPIO pins to LOW
    for(int i=0;i<16;i++)
    {
        inkplate.externalIO.digitalWrite(i, LOW);
    }
    delay(1000);                           // Wait for one second
    // Set all of the GPIO pins to HIGH
    for(int i=0;i<16;i++)
    {
        inkplate.externalIO.digitalWrite(i, HIGH);
    }
    delay(1000);                           // Wait for one second
}