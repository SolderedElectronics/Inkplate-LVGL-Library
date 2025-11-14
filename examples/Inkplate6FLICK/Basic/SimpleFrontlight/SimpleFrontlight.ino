/**
 **************************************************
 *
 * @file        SimpleFrontlight.ino
 * @brief       This example will show you how to use Inkplate 6FLICK frontlight.
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit https://soldered.com/documentation/inkplate/6flick/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE6FLICK
#error "Wrong board selection for this example, please select Soldered Inkplate 6FLICK"
#endif

#include "Inkplate.h" //Include Inkplate library

Inkplate inkplate(INKPLATE_1BIT); // Create an object on Inkplate class

int b = 31; // Variable that holds intensity of the frontlight

void setup()
{
  Serial.begin(115200);       // Set up a serial communication of 115200 baud
  inkplate.begin();           // Init Inkplate library
  inkplate.frontlight(true);  // Enable frontlight circuit
  inkplate.setFrontlight(b);  // Set frontlight intensity
}

void loop()
{
  /*
    Change frontlight value by sending "+" sign into serial monitor to increase frontlight or
    "-" sign to decrese frontlight, try to find hidden lightshow ;)
  */
  if (Serial.available()) 
  {
    bool change = false;    // Variable that indicates that frontlight value has changed and intessity has to be updated
    char c = Serial.read(); // Read incomming serial data

    if (c == '+' && b < 63) // If is received +, increase frontlight
    {
      b++;
      change = true;
    }
    if (c == '-' && b > 0) // If is received -, decrease frontlight
    {
      b--;
      change = true;
    }

    if (c == 's')
    {
      for (int j = 0; j < 4; ++j)
      {
        for (int i = 0; i < 64; ++i)
        {
          inkplate.setFrontlight(i);
          delay(30);
        }

        for (int i = 63; i >= 0; --i)
        {
          inkplate.setFrontlight(i);
          delay(30);
        }
      }
      change = true;
    }

    if (change) // If frontlight valuse has changed, update the intensity and show current value of frontlight
    {
      inkplate.setFrontlight(b);
      Serial.print("Frontlight:");
      Serial.print(b, DEC);
      Serial.println("/63");
    }
  }
}