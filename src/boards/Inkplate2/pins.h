#ifndef __INKPLATE2_PINS_H__
#define __INKPLATE2_PINS_H__
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE2

#define EPAPER_RST_PIN  19
#define EPAPER_DC_PIN   33
#define EPAPER_CS_PIN   15
#define EPAPER_BUSY_PIN 32
#define EPAPER_CLK      18
#define EPAPER_DIN      23

#define BUSY_TIMEOUT_MS 1000

#define E_INK_HEIGHT 212
#define E_INK_WIDTH  104

// in defines.h
#define INKPLATE2_WHITE 0
#define INKPLATE2_BLACK 1
#define INKPLATE2_RED   2

#ifndef _swap_int16_t
#define _swap_int16_t(a, b)                                                                                            \
    {                                                                                                                  \
        int16_t t = a;                                                                                                 \
        a = b;                                                                                                         \
        b = t;                                                                                                         \
    }
#endif

#endif
#endif