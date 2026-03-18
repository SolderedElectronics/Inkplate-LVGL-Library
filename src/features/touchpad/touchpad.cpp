#if defined(ARDUINO_INKPLATECOLOR) || defined(ARDUINO_INKPLATE6) || defined(ARDUINO_INKPLATE10)
#include "Inkplate-LVGL.h"
#include "touchpad.h"


void Touchpad::begin(Inkplate *inkplateptr)
{
    _inkplate = inkplateptr;
}

uint8_t Touchpad::read(uint8_t pad)
{
    uint8_t pin;
    switch (pad)
    {
    case 1:
        pin = 10;
        break;
    case 2:
        pin = 11;
        break;
    case 3:
        pin = 12;
        break;
    default:
        return 0;
    }
    return _inkplate->internalIO.digitalRead(pin);
}
#endif