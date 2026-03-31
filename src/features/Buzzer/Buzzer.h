/**
 **************************************************
 *
 * @file        Buzzer.h
 * @brief       Buzzer control interface for Inkplate 4TEMPERA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#ifndef __BUZZER_H__
#define __BUZZER_H__

#ifdef ARDUINO_INKPLATE4TEMPERA

#include "Arduino.h"

// Include digipot library
#include "../MCP4018/MCP4018-SOLDERED.h"

#define BEEP_FREQ_MAX 2933
#define BEEP_FREQ_MIN 572

class Inkplate;

class Buzzer
{
  public:
    Buzzer() {};
    void begin(Inkplate *_inkplatePtr);
    void init();
    void beep(uint32_t length, int freq);
    void beep(uint32_t length);
    void beepOn(int freq);
    void beepOn();
    void beepOff();

  private:
    MCP4018_SOLDERED digipot;
    Inkplate *_inkplate = NULL;
    void setFrequencyInternal(int freq);
    int freqToWiperPercent(int freq);
};

#endif

#endif