// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE4TEMPERA
#ifndef __INKPLATE4TEMPERA_PINS_H__
#define __INKPLATE4TEMPERA_PINS_H__
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#define IO_INT_ADDR 0x20
#define IO_EXT_ADDR 0x21

// Addresss to control the digipot (and affect beep frequency)
#define DIGIPOT_ADDR 0x2F

// Pin on the internal io expander which controls MOSFET for turning on and off the SD card
#define SD_PMOS_PIN 11

// Some constants used in the libraryc
#define GPIO0_ENABLE  8
#define INKPLATE_1BIT 0
#define INKPLATE_3BIT 1
#define PWR_GOOD_OK   0b11111010

// Constants for selecting a sensor when turning them on or off
#define INKPLATE_BUZZER        0x01
#define INKPLATE_ACCELEROMETER 0x02
#define INKPLATE_BME688        0x04
#define BME_CONTROL_ADDR       0x74
#define INKPLATE_APDS9960      0x08
#define INKPLATE_FUEL_GAUGE    0x1

// Pin interrupts connected to the internal gpio expander
#define INT_APDS 9
#define INT2_LSM 13
#define INT1_LSM 14
// Buzzer enable pin (pin P1_4 on internal IO expander)
#define BUZZ_EN 12

// Pin for Fuel Gauge GPOUT.
#define FG_GPOUT 15

// Frontlight
#define FRONTLIGHT_EN 10

// Touchscreen defines
#define TOUCHSCREEN_IO_EXPANDER IO_EXT_ADDR
#define TOUCHSCREEN_IO_REGS     ioRegsEx
#define TOUCHSCREEN_EN          0
#define TOUCHSCREEN_RST         1
#define TOUCHSCREEN_INT         36
#define TOUCHSCREEN_ADDR        0x15


#define WAKEUP 3
#define WAKEUP_SET                                                                                                     \
    {                                                                                                                  \
        internalIO.digitalWrite(WAKEUP, HIGH, true);                                                                   \
    }
#define WAKEUP_CLEAR                                                                                                   \
    {                                                                                                                  \
        internalIO.digitalWrite(WAKEUP, LOW, true);                                                                    \
    }
#define PWRUP 4
#define PWRUP_SET                                                                                                      \
    {                                                                                                                  \
        internalIO.digitalWrite(PWRUP, HIGH, true);                                                                    \
    }
#define PWRUP_CLEAR                                                                                                    \
    {                                                                                                                  \
        internalIO.digitalWrite(PWRUP, LOW, true);                                                                     \
    }
#define VCOM 5
#define VCOM_SET                                                                                                       \
    {                                                                                                                  \
        internalIO.digitalWrite(VCOM, HIGH, true);                                                                     \
    }
#define VCOM_CLEAR                                                                                                     \
    {                                                                                                                  \
        internalIO.digitalWrite(VCOM, LOW, true);                                                                      \
    }
#define OE 0
#define OE_SET                                                                                                         \
    {                                                                                                                  \
        internalIO.digitalWrite(OE, HIGH, true);                                                                       \
    }
#define OE_CLEAR                                                                                                       \
    {                                                                                                                  \
        internalIO.digitalWrite(OE, LOW, true);                                                                        \
    }
#define GMOD 1
#define GMOD_SET                                                                                                       \
    {                                                                                                                  \
        internalIO.digitalWrite(GMOD, HIGH, true);                                                                     \
    }
#define GMOD_CLEAR                                                                                                     \
    {                                                                                                                  \
        internalIO.digitalWrite(GMOD, LOW, true);                                                                      \
    }
#define SPV 2
#define SPV_SET                                                                                                        \
    {                                                                                                                  \
        internalIO.digitalWrite(SPV, HIGH, true);                                                                      \
    }
#define SPV_CLEAR                                                                                                      \
    {                                                                                                                  \
        internalIO.digitalWrite(SPV, LOW, true);                                                                       \
    }
#define CL 0x01
#define CL_SET                                                                                                         \
    {                                                                                                                  \
        GPIO.out_w1ts = CL;                                                                                            \
    }
#define CL_CLEAR                                                                                                       \
    {                                                                                                                  \
        GPIO.out_w1tc = CL;                                                                                            \
    }
#define CKV 0x01
#define CKV_SET                                                                                                        \
    {                                                                                                                  \
        GPIO.out1_w1ts.val = CKV;                                                                                      \
    }
#define CKV_CLEAR                                                                                                      \
    {                                                                                                                  \
        GPIO.out1_w1tc.val = CKV;                                                                                      \
    }
#define SPH 0x02
#define SPH_SET                                                                                                        \
    {                                                                                                                  \
        GPIO.out1_w1ts.val = SPH;                                                                                      \
    }
#define SPH_CLEAR                                                                                                      \
    {                                                                                                                  \
        GPIO.out1_w1tc.val = SPH;                                                                                      \
    }
#define LE 0x04
#define LE_SET                                                                                                         \
    {                                                                                                                  \
        GPIO.out_w1ts = LE;                                                                                            \
    }
#define LE_CLEAR                                                                                                       \
    {                                                                                                                  \
        GPIO.out_w1tc = LE;                                                                                            \
    }

#endif
#endif