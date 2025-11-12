#ifndef __INKPLATE6COLORDRIVER_H__
#define __INKPLATE6COLORDRIVER_H__

// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATECOLOR

// Inkplate Board name.
#define INKPLATE_BOARD_NAME "Inkplate 6COLOR"

// Include main header file for the Arduino.
#include "Arduino.h"

// Include library for PCAL6416A GPIO expander.
#include "../../system/pcalExpander/pcalExpander.h"

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

    RTC rtc;

    DitherAlgorithm dither;

    void setSdCardOk(int16_t s);
    int16_t getSdCardOk();
    int16_t sdCardInit();
    void sdCardSleep();
    SdFat getSdFat();
    SPIClass *getSPIptr();

    double readBattery();

    void clean();


    IOExpander internalIO;

    uint8_t _beginDone = 0;
    uint8_t _displayMode;

    uint8_t *DMemory4Bit;

    int16_t _sdCardOk = 0;


  private:
    void calculateLUTs();
    void pmicBegin();
    uint8_t initializeFramebuffers();
    void gpioInit();
    uint8_t getPanelState();
    void setPanelState(uint8_t state);
    
    void resetPanel();
    void sendCommand(uint8_t _command);
    void sendData(uint8_t *_data, int _n);
    void sendData(uint8_t _data);
    bool setPanelDeepSleep(bool _state);
    void setIOExpanderForLowPower();
    uint8_t _panelState = 0;
    Inkplate *_inkplate;

    // Color palette of the Specta screen.
    uint16_t _paletteIdeal[7] = {0x0000, 0xFFFF, 0x07E0, 0x001F, 0xF800, 0xFFE0, 0xFBE0};
    // uint16_t _palleteMeasured[6] = {0x4A4A, 0xA554, 0xB549, 0x7249, 0x4B32, 0x536C};
    uint8_t _paletteIndex[7] = {0, 1, 2, 3, 4, 5, 6};

    int8_t paletteSize = 7;
};

#endif
#endif