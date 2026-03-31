/**
 **************************************************
 *
 * @file        Inkplate5V2BoardFile.h
 * @brief       Board wrapper for Inkplate 5V2
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro @ Soldered
 ***************************************************/


// Header guard.
#ifndef __INKPLATE5V2_BOARD_SELECT_H__
#define __INKPLATE5V2_BOARD_SELECT_H__

// Board select check.
#ifdef ARDUINO_INKPLATE5V2

// Include Inkplate10 board header file.
#include "Inkplate5V2Driver.h"

// Wrapper for different Inkplate boards.
class InkplateBoardClass : public EPDDriver
{
  public:
    InkplateBoardClass() {};
};

#endif
#endif