/**
 **************************************************
 *
 * @file        waveforms.h
 * @brief       EPD waveform lookup tables for Inkplate 6FLICK
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#ifndef __WAVEFROMS_INKPLATE_6FLICK_H__
#define __WAVEFROMS_INKPLATE_6FLICK_H__

#ifdef ARDUINO_INKPLATE6FLICK

#define WAVEFORM3BIT                                                                                                   \
    {{0, 0, 0, 0, 0, 1, 1, 1, 0}, {0, 0, 1, 2, 1, 1, 2, 1, 0}, {0, 1, 1, 2, 1, 1, 1, 2, 0},                            \
     {1, 1, 1, 2, 2, 1, 1, 2, 0}, {1, 1, 1, 2, 1, 2, 1, 2, 0}, {0, 1, 1, 2, 1, 2, 1, 2, 0},                            \
     {1, 2, 1, 1, 2, 2, 1, 2, 0}, {0, 0, 0, 0, 0, 0, 0, 2, 0}};


#define E_INK_WIDTH  1024
#define E_INK_HEIGHT 758
#endif
#endif
