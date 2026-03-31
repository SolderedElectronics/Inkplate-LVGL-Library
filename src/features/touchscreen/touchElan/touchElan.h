/**
 **************************************************
 *
 * @file        touchElan.h
 * @brief       Elan touchscreen controller interface,
 *              used by 4TEMPERA and 6PLUS devices
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/


#ifndef __TOUCH_ELAN_H__
#define __TOUCH_ELAN_H__

#if defined(ARDUINO_INKPLATE6PLUS) || defined(ARDUINO_INKPLATE6PLUSV2) || defined(ARDUINO_INKPLATE4TEMPERA)

// Include Arduino main header file.
#include "Arduino.h"

// Include Arduino Wire library (for I2C communication with touchscreen driver).
#include "Wire.h"

// Include board specific defines.
#include "../../../system/defines.h"

class Inkplate;

/**
 * @brief       Touch class holds functionality for interaction with touchscreen
 * displays
 */
class Touch
{
  public:
    bool touchInArea(int16_t x1, int16_t y1, int16_t w, int16_t h);
    void begin(Inkplate *inkplatePtr);
    bool init(uint8_t _pwrState);
    void shutdown();
    bool available();
    void setPowerState(uint8_t _s);
    uint8_t getPowerState();
    uint8_t getData(uint16_t *xPos, uint16_t *yPos);
    void getRawData(uint8_t *b);

  private:
    const char hello_packet[4] = {0x55, 0x55, 0x55, 0x55};

    uint8_t tsWriteRegs(uint8_t _addr, const uint8_t *_buff, uint8_t _size);
    void tsReadRegs(uint8_t _addr, uint8_t *_buff, uint8_t _size);
    void power(bool _pwr);
    void tsHardwareReset();
    bool tsSoftwareReset();
    void tsGetXY(uint8_t *_d, uint16_t *x, uint16_t *y);
    void tsGetResolution(uint16_t *xRes, uint16_t *yRes);
    void end();

    uint8_t touchN;
    uint16_t touchX[2], touchY[2];
    uint32_t touchT = 0;
    bool _tsInitDone = false;

    Inkplate *_inkplate;
};

#endif

#endif