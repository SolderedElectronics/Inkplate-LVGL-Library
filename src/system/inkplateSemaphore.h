/**
 **************************************************
 *
 * @file        inkplateSemaphore.h
 * @brief       FreeRTOS mutex handles and convenience macros for
 *              thread-safe I2C and SPI access on Inkplate 4TEMPERA.
 *
 *              mutexI2C and mutexSPI are defined in the active board
 *              Inkplate*Driver.cpp and created in initDriver(). All driver and feature code
 *              that touches the Wire bus or SPI bus includes this header
 *              and wraps every transaction with i2cStart()/i2cEnd() or
 *              spiStart()/spiEnd().
 *
 *              Recursive mutexes allow nested i2cStart (e.g. driver + PCAL
 *              expander) without deadlocking. On non-Inkplate builds the macros
 *              expand to ((void)0).
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t mutexI2C;
extern SemaphoreHandle_t mutexSPI;

#define i2cStart() xSemaphoreTakeRecursive(mutexI2C, portMAX_DELAY)
#define i2cEnd()   xSemaphoreGiveRecursive(mutexI2C)
#define spiStart() xSemaphoreTakeRecursive(mutexSPI, portMAX_DELAY)
#define spiEnd()   xSemaphoreGiveRecursive(mutexSPI)

