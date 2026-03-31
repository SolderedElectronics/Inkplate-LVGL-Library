/**
 **************************************************
 *
 * @file        Inkplate6FLICKBoardFile.h
 * @brief       Board wrapper for Inkplate 6FLICK
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro @ Soldered
 ***************************************************/


// Header guard.
#ifndef __INKPLATE6FLICK_BOARD_SELECT_H__
#define __INKPLATE6FLICK_BOARD_SELECT_H__

// Board select check.
#ifdef ARDUINO_INKPLATE6FLICK

// Include Inkplate6 board header file.
#include "Inkplate6FLICKDriver.h"

// Wrapper for different Inkplate boards.
class InkplateBoardClass : public EPDDriver
{
  public:
    InkplateBoardClass() {};
};

#endif
#endif