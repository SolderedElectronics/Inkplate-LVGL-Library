/**
 **************************************************
 *
 * @file        BME680-SOLDERED.cpp
 * @brief       BME688 sensor wrapper for Inkplate4TEMPERA using Bosch bme68x driver
 *
 *
 * @copyright GNU General Public License v3.0
 * @authors     Zvonimir Haramustek for Soldered.com
 ***************************************************/

#ifdef ARDUINO_INKPLATE4TEMPERA

#include "BME680-SOLDERED.h"
#include "../../system/inkplateSemaphore.h"

static const uint16_t HEATER_TEMP_DEG = 320;
static const uint16_t HEATER_DUR_MS = 150;

bool BME680::begin()
{
    i2cStart();
    Bme68x::begin(BME68X_I2C_ADDR_LOW, Wire);
    bool ok = checkStatus() != BME68X_ERROR;
    if (ok)
    {
        setTPH(BME68X_OS_16X, BME68X_OS_16X, BME68X_OS_16X);
        setFilter(BME68X_FILTER_SIZE_3);
        setHeaterProf(HEATER_TEMP_DEG, HEATER_DUR_MS);
        ok = checkStatus() != BME68X_ERROR;
    }
    i2cEnd();
    return ok;
}

bool BME680::readData(bme68xData &data)
{
    setOpMode(BME68X_FORCED_MODE);
    delayMicroseconds(getMeasDur(BME68X_FORCED_MODE) + (uint32_t)HEATER_DUR_MS * 1000UL);
    uint8_t n = fetchData();
    if (n > 0)
    {
        getData(data);
        return true;
    }
    return false;
}

float BME680::readTemperature()
{
    bme68xData data = {};
    i2cStart();
    readData(data);
    i2cEnd();
    return data.temperature;
}

float BME680::readPressure()
{
    bme68xData data = {};
    i2cStart();
    readData(data);
    i2cEnd();
    return data.pressure / 100.0f;
}

float BME680::readHumidity()
{
    bme68xData data = {};
    i2cStart();
    readData(data);
    i2cEnd();
    return data.humidity;
}

float BME680::readAltitude()
{
    bme68xData data = {};
    i2cStart();
    readData(data);
    i2cEnd();
    float seaLevel = 1013.25f;
    return 44330.0f * (1.0f - powf(data.pressure / 100.0f / seaLevel, 0.1903f));
}

float BME680::readGasResistance()
{
    bme68xData data = {};
    i2cStart();
    readData(data);
    i2cEnd();
    return data.gas_resistance / 100.0f;
}

void BME680::readSensorData(float &temp, float &humidity, float &pressure, float &gas)
{
    bme68xData data = {};
    i2cStart();
    readData(data);
    i2cEnd();
    temp = data.temperature;
    humidity = data.humidity;
    pressure = data.pressure / 100.0f;
    gas = data.gas_resistance / 100.0f;
}

float BME680::calculateAltitude(float pressure)
{
    float seaLevel = 1013.25f;
    return 44330.0f * (1.0f - powf(pressure / seaLevel, 0.1903f));
}

#endif
