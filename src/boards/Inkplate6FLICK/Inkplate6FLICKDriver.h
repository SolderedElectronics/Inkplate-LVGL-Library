#ifndef __INKPLATE6FLICKDRIVER_H__
#define __INKPLATE6FLICKDRIVER_H__

// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE6FLICK

// Inkplate Board name.
#define INKPLATE_BOARD_NAME "Inkplate 6FLICK"

// Include main header file for the Arduino.
#include "Arduino.h"

// Include library for PCAL6416A GPIO expander.
#include "../../system/pcalExpander/pcalExpander.h"

#include "pins.h"

// Include waveforms for EPD
#include "waveforms.h"


#include "Wire.h"

#include "../../graphics/GraphicsDefs.h"

#include "../../features/featureSelect.h"

#include "../../system/defines.h"

#include "../../system/esp32/Esp.h"

#include "../../graphics/ditheringGrayscale/ditherAlgorithm.h"


class Inkplate;


class EPDDriver : public Esp
{
  public:
    int initDriver(Inkplate *_inkplatePtr);
    void IRAM_ATTR writePixelInternal(int16_t x, int16_t y, uint16_t color);
    void display(bool _leaveOn = 0);
    void selectDisplayMode(uint8_t displayMode);
    void clearDisplay();
    uint32_t partialUpdate(bool _forced = false, bool leaveOn = false);
    void setFullUpdateThreshold(uint16_t _numberOfPartialUpdates);
    uint8_t getDisplayMode();


    void setSdCardOk(int16_t s);
    int16_t getSdCardOk();
    int16_t sdCardInit();
    void sdCardSleep();
    SdFat getSdFat();
    SPIClass *getSPIptr();

    int8_t readTemperature();

    double readBattery();

    DitherAlgorithm dither;

    IOExpander internalIO;
    IOExpander externalIO;

    RTC rtc;

    Touch touchscreen;

    Frontlight frontlight;

    uint8_t _beginDone = 0;
    uint8_t _displayMode;


    uint32_t pinLUT[256];
    uint32_t *GLUT;
    uint32_t *GLUT2;
    uint8_t *DMemoryNew;
    uint8_t *_partial;
    uint8_t *DMemory4Bit;
    uint8_t *_pBuffer;
    uint8_t waveform3Bit[8][9] = WAVEFORM3BIT;
    uint16_t _partialUpdateLimiter = 10;
    uint16_t _partialUpdateCounter = 0;
    uint8_t _blockPartial = 1;
    int16_t _sdCardOk = 0;


  private:
    void calculateLUTs();
    void pmicBegin();
    uint8_t initializeFramebuffers();
    void gpioInit();
    uint8_t readPowerGood();
    void pinsAsOutputs();
    int einkOn();
    void einkOff();
    void display1b(bool _leaveOn);
    void display3b(bool _leaveOn);
    void pinsZstate();
    uint8_t getPanelState();
    void setPanelState(uint8_t state);
    void clean(uint8_t c, uint8_t rep);
    void vscan_start();
    void hscan_start(uint32_t _d);
    void vscan_end();
    uint8_t _panelState = 0;
    Inkplate *_inkplate;
};

#endif
#endif