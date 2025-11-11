/**
 **************************************************
 *
 * @file        boardSelect.h
 * @brief       Main header file of the Inkpate board selector used to
 *              include proper files for the selected Inkplate board.
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Borna Biro for soldered.com
 ***************************************************/

#ifndef __MY_TEST_LIBRARY_BOARD_SELECT_H__
#define __MY_TEST_LIBRARY_BOARD_SELECT_H__

#include <Arduino.h>

#ifdef ARDUINO_INKPLATE10V2
#include "boards/Inkplate10/Inkplate10BoardFile.h"
#elif defined(ARDUINO_INKPLATE6V2)
#define USES_I2S
#include "boards/Inkplate6/Inkplate6BoardFile.h"
#elif defined(ARDUINO_INKPLATE6FLICK)
#define USES_I2S
#define MULTIPLE_DISPLAY_MODES
#include "boards/Inkplate6FLICK/Inkplate6FLICKBoardFile.h"
#elif defined(ARDUINO_INKPLATE5V2)
#define USES_I2S
#define MULTIPLE_DISPLAY_MODES
#include "boards/Inkplate5V2/Inkplate5V2BoardFile.h"
#elif defined(ARDUINO_INKPLATECOLOR)
#define USE_COLOR_IMAGE
#include "boards/Inkplate6COLOR/Inkplate6COLORBoardFile.h"
#elif defined(ARDUINO_INKPLATE2)
#define USE_COLOR_IMAGE
#include "boards/Inkplate2/Inkplate2BoardFile.h"
#else
#error "Board not selected!"
#endif

#endif