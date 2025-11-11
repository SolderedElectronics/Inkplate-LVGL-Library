#ifndef __INKPLATE2DRIVER_H__
#define __INKPLATE2DRIVER_H__

// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE2

// Inkplate Board name.
#define INKPLATE_BOARD_NAME "Inkplate 2"

// Include main header file for the Arduino.
#include "Arduino.h"

#include "pins.h"

#include "Wire.h"

#include "../../graphics/GraphicsDefs.h"

#include "../../features/featureSelect.h"

#include "../../system/defines.h"

#include "../../graphics/ditheringColor/ditherAlgorithm.h"

class Inkplate;


class EPDDriver
{
  public:
    void writePixelInternal(int16_t x, int16_t y, uint16_t color);
    int initDriver(Inkplate *_inkplatePtr);

    void display(bool _leaveOn = 0);
    void selectDisplayMode(uint8_t displayMode);
    void clearDisplay();

    double readBattery();

    uint8_t _beginDone = 0;
    uint8_t _displayMode;

    DitherAlgorithm dither;

    uint8_t *DMemory4Bit;


  private:
    // Color palette of the Inkplate 2 screen
    uint16_t _paletteIdeal[3] = {0xFFFF, 0x0000, 0xF800};

    uint8_t _paletteIndex[3] = {0, 1, 2};

    int8_t paletteSize = 3;

    uint8_t getPanelState();
    void setPanelState(uint8_t state);
    bool waitForEpd(uint16_t _timeout);
    void clean();
    void resetPanel();
    void sendCommand(uint8_t _command);
    void sendData(uint8_t *_data, int _n);
    void sendData(uint8_t _data);
    bool setPanelDeepSleep(bool _state);
    uint8_t _panelState = 0;
    Inkplate *_inkplate;
};

#endif
#endif