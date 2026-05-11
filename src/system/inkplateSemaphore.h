/**
 **************************************************
 *
 * @file        inkplateSemaphore.h
 * @brief       FreeRTOS mutex handles and convenience macros for
 *              thread-safe I2C, SPI and display access on Inkplate boards.
 *
 *              All three mutexes are defined in inkplateSemaphore.cpp and
 *              created via inkplateMutexInit() called from Inkplate::begin().
 *              All driver and feature code that touches Wire, SPI or the
 *              display framebuffer includes this header and wraps every
 *              transaction with the matching start/end macro pair.
 *
 *              Recursive mutexes allow nested locks (e.g. driver + PCAL
 *              expander) without deadlocking.
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#pragma once

#include "freertos/FreeRTOS.h"
#include "freertos/semphr.h"

extern SemaphoreHandle_t mutexI2C;
extern SemaphoreHandle_t mutexSPI;
extern SemaphoreHandle_t mutexDisplay;

void inkplateMutexInit();

#define i2cStart()     xSemaphoreTakeRecursive(mutexI2C, portMAX_DELAY)
#define i2cEnd()       xSemaphoreGiveRecursive(mutexI2C)
#define spiStart()     xSemaphoreTakeRecursive(mutexSPI, portMAX_DELAY)
#define spiEnd()       xSemaphoreGiveRecursive(mutexSPI)
#define displayStart() xSemaphoreTakeRecursive(mutexDisplay, portMAX_DELAY)
#define displayEnd()   xSemaphoreGiveRecursive(mutexDisplay)
