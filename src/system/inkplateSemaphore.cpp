/**
 **************************************************
 *
 * @file        inkplateSemaphore.cpp
 * @brief       FreeRTOS mutex definitions and initialisation for thread-safe
 *              I2C, SPI and display access on Inkplate boards.
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

#include "inkplateSemaphore.h"

SemaphoreHandle_t mutexI2C = NULL;
SemaphoreHandle_t mutexSPI = NULL;
SemaphoreHandle_t mutexDisplay = NULL;

void inkplateMutexInit()
{
    mutexI2C = xSemaphoreCreateRecursiveMutex();
    mutexSPI = xSemaphoreCreateRecursiveMutex();
    mutexDisplay = xSemaphoreCreateRecursiveMutex();
}
