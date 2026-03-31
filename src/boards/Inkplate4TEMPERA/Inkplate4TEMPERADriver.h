/**
 **************************************************
 *
 * @file        Inkplate4TEMPERADriver.h
 * @brief       Low-level EPD driver class definition for Inkplate 4TEMPERA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE4TEMPERA

#ifndef __INKPLATE4TEMPERADRIVER_H__
#define __INKPLATE4TEMPERADRIVER_H__

// Inkplate Board name.
#define INKPLATE_BOARD_NAME "Inkplate 4TEMPERA"

// Include main header file for the Arduino.
#include "Arduino.h"

#include "EEPROM.h"

// Include library for PCAL6416A GPIO expander.
#include "../../system/pcalExpander/pcalExpander.h"

#include "pins.h"

// Include waveforms for EPD
#include "waveforms.h"

#include "../../graphics/ditheringGrayscale/ditherAlgorithm.h"

#include "Wire.h"

#include "../../graphics/GraphicsDefs.h"

#include "../../features/featureSelect.h"

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
    uint32_t partialUpdate(bool _forced = false, bool leaveOn = false);
    void setFullUpdateThreshold(uint16_t _numberOfPartialUpdates);
    uint8_t getDisplayMode();

    int einkOn();
    void einkOff();

    void setSdCardOk(int16_t s);
    int16_t getSdCardOk();
    int16_t sdCardInit();
    void sdCardSleep();
    SdFat &getSdFat();
    SPIClass *getSPIptr();

    void wakePeripheral(uint8_t _peripheral);
    void sleepPeripheral(uint8_t _peripheral);

    int8_t readTemperature();

    void burnInClean(uint8_t clear_cycles, uint16_t cycles_delay);

    bool setVCOM(double vcom);
    double getStoredVCOM();
    double getVCOMValue();

    bool isPowerGood();

    double getVcomVoltage();

    IOExpander internalIO;
    IOExpander externalIO;

    RTC rtc;

    DitherAlgorithm dither;

    APDS_9960 apds9960;       // Gesture sensor
    BME680 bme688;            // Temperature, pressure and humidity sensor
    Soldered_LSM6DS3 lsm6ds3; // Accelerometer/Gyroscope
    BQ_27441 battery;         // Fuel gauge
    Buzzer buzzer;

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
    uint8_t waveform3Bit[8][8] = WAVEFORM3BIT;
    uint16_t _partialUpdateLimiter = 10;
    uint16_t _partialUpdateCounter = 0;
    uint8_t _blockPartial = 1;
    int16_t _sdCardOk = 0;


  private:
    void calculateLUTs();
    void pmicBegin();
    void blockGpioPins();
    uint8_t initializeFramebuffers();
    void gpioInit();
    uint8_t readPowerGood();
    void pinsAsOutputs();
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
    volatile bool _renderReady = false;
    volatile bool _noRender = false;
    static void _renderReadyCb(lv_event_t *e);
    static void _refrReadyCb(lv_event_t *e);
    bool writeVCOMToPanelEEPROM(double vcom);
    void writeReg(uint8_t reg, float data);
    uint8_t readReg(uint8_t reg);
};
#endif
#endif