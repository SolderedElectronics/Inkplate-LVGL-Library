/**
 **************************************************
 * @file        UtilI2S.h
 * @brief       File for ESP I2S utility, class declaration
 *
 * @authors     Soldered
 ***************************************************/

#ifndef __UTILI2S_H__
#define __UTILI2S_H__

#include "Arduino.h"
#include "rom/lldesc.h"
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#include "soc/i2s_reg.h"
#include "soc/i2s_struct.h"
#include "soc/rtc.h"
#include "soc/soc.h"

#include "driver/i2s_std.h"


/**
 * @brief       I2S class used to send data to the panel via the I2S1 LCD
 *              parallel mode.  The I2S1 peripheral of the original ESP32 is
 *              used as a high-speed 8-bit parallel output bus; this is
 *              sometimes called the "I2S LCD trick".
 */
class UtilI2S
{
  public:
    void IRAM_ATTR I2SInit(volatile i2s_dev_t *_i2sDev, uint8_t _clockDivider = 5);
    void IRAM_ATTR sendDataI2S(volatile i2s_dev_t *_i2sDev, volatile lldesc_s *_dmaDecs);
    void IRAM_ATTR setI2S1pin(uint32_t _pin, uint32_t _function, uint32_t _inv);

  protected:
    volatile uint8_t *_dmaLineBuffer;
    volatile lldesc_s *_dmaI2SDesc;

    // Use only I2S1 (I2S0 is not compatible with 8-bit data).
    volatile i2s_dev_t *myI2S;

  private:
    // Channel handle returned by i2s_new_channel(); kept so the peripheral
    // stays claimed for the lifetime of the driver object.
    i2s_chan_handle_t _i2sChanHandle = NULL;
};

#endif
