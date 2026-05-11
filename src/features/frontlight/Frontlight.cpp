/**
 **************************************************
 *
 * @file        Frontlight.cpp
 * @brief       Frontlight control implementation for Inkplate boards
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/


#include "Frontlight.h"
#include "Inkplate-LVGL.h"

#if defined(ARDUINO_INKPLATE6PLUS) || defined(ARDUINO_INKPLATE6PLUSV2) || defined(ARDUINO_INKPLATE4TEMPERA) ||         \
    defined(ARDUINO_INKPLATE6FLICK)

/**
 * @brief       setFrontlight function sets frontlight intensity for inkplate
 *
 * @param       uint8_t _v
 *              value to set frontlight to
 *
 * @note        can only be used in inkplate 6PLUS and 4TEMPERA, others don't suport
 * frontlight
 */
void Frontlight::setBrightness(uint8_t _v)
{
    i2cStart();
    Wire.beginTransmission(0x5C >> 1);
    Wire.write(0);
    Wire.write(63 - (_v & 0b00111111));
    Wire.endTransmission();
    i2cEnd();
}

/**
 * @brief       setState function turns frontlight on/off
 *
 * @param       bool _e
 *              enable value, 1 turns on, 0 off
 */
void Frontlight::setState(bool _e)
{
    if (_e)
    {
        _inkplate->expander1.digitalWrite(FRONTLIGHT_EN, HIGH);
    }
    else
    {
        _inkplate->expander1.digitalWrite(FRONTLIGHT_EN, LOW);
    }
}

/**
 * @brief       begin function forwards the current inkplate instance to be used inside the class
 *
 * @param       Inkplate* inkplatePtr
 *              pointer to the instance of the current Inkplate display object
 */
void Frontlight::begin(Inkplate *inkplatePtr)
{
    _inkplate = inkplatePtr;
}

#endif