/**
 **************************************************
 *
 * @file        Inkplate4TEMPERABoardFile.h
 * @brief       Board wrapper for Inkplate 4TEMPERA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro @ Soldered
 ***************************************************/


// Header guard.
#ifndef __INKPLATE4TEMPERA_BOARD_SELECT_H__
#define __INKPLATE4TEMPERA_BOARD_SELECT_H__

// Board select check.
#ifdef ARDUINO_INKPLATE4TEMPERA

// Include Inkplate10 board header file.
#include "Inkplate4TEMPERADriver.h"

// Wrapper for different Inkplate boards.
class InkplateBoardClass : public EPDDriver
{
  public:
    InkplateBoardClass() {};
};

#endif
#endif