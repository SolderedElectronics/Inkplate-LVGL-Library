/**
 **************************************************
 *
 * @file        BME680-SOLDERED.h
 * @brief       Header file for BME688 environmental sensor (Inkplate4TEMPERA)
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Zvonimir Haramustek for Soldered.com
 ***************************************************/

#ifdef ARDUINO_INKPLATE4TEMPERA

#ifndef __BME680_SOLDERED__
#define __BME680_SOLDERED__

#include "Arduino.h"
#include "libs/Bosch-BME68x-Library/bme68x/bme68x_defs.h"
#include "libs/Bosch-BME68x-Library/bme68xLibrary.h"

class BME680 : public Bme68x
{
  public:
    bool begin();

    float readTemperature();
    float readPressure();
    float readHumidity();
    float readAltitude();
    float readGasResistance();
    void readSensorData(float &temp, float &humidity, float &pressure, float &gas);

    float calculateAltitude(float pressure);

  protected:
    void initializeNative(){};

  private:
    bool readData(bme68xData &data);
};

#endif

#endif
