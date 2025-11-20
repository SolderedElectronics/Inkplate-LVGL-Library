/**
 **************************************************
 *
 * @file        Frontlight.ino
 * @brief       This example will show you how to use frontlight feature on Inkplate 6FLICK.
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit
 * https://soldered.com/documentation/inkplate/6flick/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display in 1-bit mode (change to INKPLATE_3BIT if you want grayscale)
Inkplate inkplate(INKPLATE_1BIT);

// Set initial brightness value
int b = 31;

void setup() 
{
  Serial.begin(115200);

  // Initialize inkplate
  inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);

  // Turn frontlight ON
  inkplate.frontlight.setState(true);
  // Set initial frontlight brightness intensity
  inkplate.frontlight.setBrightness(b);
}

void loop() 
{
  // Change frontlight value by sending "+" sign into serial monitor to increase frontlight or 
  // "-" sign to decrese frontlight, try to find hidden lightshow ;)
  if (Serial.available()) 
  {
    bool change = false;      // Variable that indicates that frontlight value has changed and intessity has to be updated
    char c = Serial.read();   // Read incomming serial data

    if (c == '+' && b < 63) // If '+' received, increase frontlight
    {
      b++;
      change = true;
    }
    if (c == '-' && b > 0) // If '-' received, decrease frontlight
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
          inkplate.frontlight.setBrightness(i);
          delay(30);
        }
        for (int i = 63; i >= 0; --i)
        {
          inkplate.frontlight.setBrightness(i);
          delay(30);
        }
      }

      change = true;
    }

    if (change) // If frontlight valuse has changed, update the intensity and show current value of frontlight
    {
      inkplate.frontlight.setBrightness(b);
      Serial.print("Frontlight:");
      Serial.print(b, DEC);
      Serial.println("/63");
    }
  }
}
