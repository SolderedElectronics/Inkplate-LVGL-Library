/**
 **************************************************
 *
 * @file        EEPROMUsage.ino
 * @brief       Example showing how to clear, write to and read from the onboard EEPROM
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include "EEPROM.h"   // Include ESP32 EEPROM library
#include "Inkplate-LVGL.h" // Include Inkplate library to the sketch

#define EEPROM_SIZE 128 // How much data to write to EEPROM in this example

Inkplate inkplate; // Create object on Inkplate library and set library to work in monochrome mode

void setup()
{
    Serial.begin(115200);                 // Init serial monitor to display what's happening
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);                      // Init library (you should call this function ONLY ONCE)

    // Init EEPROM library with 128 of EEPROM size.
    EEPROM.begin(EEPROM_SIZE);

    Serial.println("Cleaning EEPROM..."); // Print message on serial monitor
    clearEEPROM();                        // Clear user EEPROM data
    delay(500);                           // Wait a little bit...

    Serial.println("Writing data to EEPROM..."); // Print message on serial monitor
    writeEEPROM();                               // Write some data to EEPROM
    delay(500);                                  // Wait a little bit...

    Serial.println("Reading data from EEPROM...");  // Print message on the Sserial Monitor
    printEEPROM();                                  // Read data from EEPROM and display it on screen
    delay(500);                                     // Wait a little bit...
}

void loop()
{
    // Empty...
} 

// Function for clearing EEPROM data
void clearEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

// Function writes data to EEPROM
void writeEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, i); 
    }
    EEPROM.commit();
}

// Function reads back previously written data and displays it on serial monitor and screen.
void printEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        Serial.print(EEPROM.read(i), DEC);
        if (i != EEPROM_SIZE - 1)
        {
            Serial.print(", ");
        }
    }
}