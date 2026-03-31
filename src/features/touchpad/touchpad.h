/**
 **************************************************
 *
 * @file        touchpad.h
 * @brief       Touchpad interface for legacy Inkplate boards
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#if defined(ARDUINO_INKPLATECOLOR) || defined(ARDUINO_INKPLATE6) || defined(ARDUINO_INKPLATE10)
#pragma once
#include "Arduino.h"

class Inkplate;

class Touchpad
{
  public:
    void begin(Inkplate *inkplateptr);
    uint8_t read(uint8_t pad);

  private:
    Inkplate *_inkplate;
};
#endif