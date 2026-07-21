/**
 **************************************************
 *
 * @file        Inkplate7SPECTRABoardFile.h
 * @brief       Board wrapper for Inkplate 7SPECTRA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/


// Header guard.
#ifndef __INKPLATE7SPECTRA_BOARD_SELECT_H__
#define __INKPLATE7SPECTRA_BOARD_SELECT_H__

// Board select check.
#ifdef ARDUINO_ESP32S3_DEV

// Include Inkplate 7SPECTRA board header file.
#include "Inkplate7SPECTRADriver.h"

// Wrapper for different Inkplate boards.
class InkplateBoardClass : public EPDDriver
{
  public:
    InkplateBoardClass() {};
};

#endif
#endif
