/**
 **************************************************
 *
 * @file        Inkplate13BoardFile.h
 * @brief       Board wrapper for Inkplate 13SPECTRA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Josip Šimun Kuči @ Soldered
 ***************************************************/


// Header guard.
#ifndef __INKPLATE13_BOARD_SELECT_H__
#define __INKPLATE13_BOARD_SELECT_H__

// Board select check.
#ifdef ARDUINO_INKPLATE13SPECTRA

// Include Inkplate10 board header file.
#include "Inkplate13Driver.h"

// Wrapper for different Inkplate boards.
class InkplateBoardClass : public EPDDriver
{
  public:
    InkplateBoardClass() {};
};

#endif
#endif