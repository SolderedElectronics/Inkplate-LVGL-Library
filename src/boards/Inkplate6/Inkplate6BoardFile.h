/**
 **************************************************
 *
 * @file        Inkplate6BoardFile.h
 * @brief       Wrrapper for the different Inkplate boards and
 *              it's classes.
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro for soldered.com
 ***************************************************/

// Header guard.
#ifndef __INKPLATE6_BOARD_SELECT_H__
#define __INKPLATE6_BOARD_SELECT_H__

// Board select check.
#ifdef ARDUINO_INKPLATE6V2

// Include Inkplate6 board header file.
#include "Inkplate6Driver.h"

// Wrapper for different Inkplate boards.
class InkplateBoardClass : public EPDDriver
{
  public:
    InkplateBoardClass(){};
};

#endif
#endif