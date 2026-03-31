/**
 **************************************************
 *
 * @file        Frontlight.h
 * @brief       Frontlight control interface for Inkplate boards
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#ifndef __FRONTLIGHT_H__
#define __FRONTLIGHT_H__

#if defined(ARDUINO_INKPLATE6PLUS) || defined(ARDUINO_INKPLATE6PLUSV2) || defined(ARDUINO_INKPLATE4TEMPERA) ||         \
    defined(ARDUINO_INKPLATE6FLICK)

#include "Arduino.h"
#include "Wire.h"

class Inkplate;

/**
 * @brief       Frontlight class designed to work with inkplates that support
 * frontlight (Inkplate 6 plus and Inkplate 4TEMPERA)
 */
class Frontlight
{
  public:
    void setBrightness(uint8_t _v);
    void setState(bool _e);
    void begin(Inkplate *inkplatePtr);

  private:
    Inkplate *_inkplate;
};

#endif

#endif