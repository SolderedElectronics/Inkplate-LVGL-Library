/**
 **************************************************
 *
 * @file        Inkplate7SPECTRADriver.h
 * @brief       Low-level EPD driver class definition for Inkplate 7SPECTRA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#ifndef __INKPLATE7SPECTRA_DRIVER_H__
#define __INKPLATE7SPECTRA_DRIVER_H__

// Header guard for the Arduino include
#ifdef ARDUINO_ESP32S3_DEV

// Inkplate Board name.
#define INKPLATE_BOARD_NAME "Inkplate 7SPECTRA"

// Include main header file for the Arduino.
#include "Arduino.h"

// Include library for PCAL6416A GPIO expander.
#include "../../system/pcalExpander/pcalExpander.h"

#include "pins.h"


#include "Wire.h"

#include "../../graphics/GraphicsDefs.h"

#include "../../features/featureSelect.h"

#include "../../graphics/ditheringColor/ditherAlgorithm.h"

#include "../../system/defines.h"


class Inkplate;


class EPDDriver
{
  public:
    void writePixelInternal(int16_t x, int16_t y, uint16_t color);
    int initDriver(Inkplate *_inkplatePtr);

    void display(bool _leaveOn = 0);
    void selectDisplayMode(uint8_t displayMode);
    void clearDisplay();


    void setSdCardOk(int16_t s);
    int16_t getSdCardOk();
    int16_t sdCardInit();
    void sdCardSleep();
    SdFat &getSdFat();
    SPIClass *getSPIptr();

    double readBattery();

    DitherAlgorithm dither;

    IOExpander expander1;

    RTC rtc;

    uint8_t _beginDone = 0;
    uint8_t _displayMode;

    uint8_t *DMemory4Bit;

    int16_t _sdCardOk = 0;


  private:
    uint8_t initializeFramebuffers();
    void screenInit();
    uint8_t getPanelState();
    void setIO();
    void setPanelState(uint8_t state);
    void resetPanel();
    void sendCommand(uint8_t _cmd, const uint8_t *_parameters, uint32_t _n);
    void waitForBusy();
    void setPanelPinsToLow();
    uint8_t _panelState = 0;
    Inkplate *_inkplate;


    // Color display of the spectra panel
    uint16_t _palette[6] = {
        0x0000, // Black
        0xFFFF, // White
        0xFFE0, // Yellow
        0xF800, // Red
        0x001F, // Blue
        0x07E0  // Green
    };


    uint8_t _paletteIndex[6] = {0, 1, 2, 3, 5, 6};

    int8_t paletteSize = 6;
};


#endif
#endif
