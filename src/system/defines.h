/**
 **************************************************
 *
 * @file        defines.h
 * @brief       Global defines and macros for Inkplate control
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/


#ifndef __DEFINES_H__
#define __DEFINES_H__

#include "Arduino.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"

// Values loaded into the EEPROM of some models if they require specific waveforms
#define INKPLATE6_WAVEFORM1     0
#define INKPLATE10_WAVEFORM1    20
#define INKPLATE10_WAVEFORM2    21
#define INKPLATE10_WAVEFORM3    22
#define INKPLATE10_WAVEFORM4    23
#define INKPLATE10_WAVEFORM5    24
#define INKPLATE6PLUS_WAVEFORM1 40

#if !defined(ARDUINO_INKPLATECOLOR) && !defined(ARDUINO_INKPLATE2) && !defined(ARDUINO_INKPLATE13SPECTRA) &&           \
    !defined(ARDUINO_ESP32S3_DEV)
#define BLACK 1
#define WHITE 0
#else
#define BLACK 0
#define WHITE 1
#endif

#define INKPLATE_1BIT          0
#define INKPLATE_3BIT          1
#define PWR_GOOD_OK            0b11111010
#define INKPLATE_FORCE_PARTIAL true


#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                                                            \
    {                                                                                                                  \
        int16_t t = a;                                                                                                 \
        a = b;                                                                                                         \
        b = t;                                                                                                         \
    }
#endif

#define BOUND(a, b, c) ((a) <= (b) && (b) <= (c))

#define GPIO0_ENABLE 8

#define DATA 0x0E8C0030

#endif
