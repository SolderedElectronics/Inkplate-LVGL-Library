#ifndef __INKPLATE10V2_PINS_H__
#define __INKPLATE10V2_PINS_H__
// Board select check.
#ifdef ARDUINO_INKPLATE10V2
#include "soc/gpio_reg.h"
#include "soc/gpio_struct.h"
#define IO_INT_ADDR 0x20
#define IO_EXT_ADDR 0x21

// Pin on the internal io expander which controls MOSFET for turning on and off the SD card
#define SD_PMOS_PIN IO_PIN_B2 // 10

#define WAKEUP 3
#define WAKEUP_SET                                                                                                     \
    {                                                                                                                  \
        internalIO.digitalWrite(WAKEUP, HIGH);                                                                         \
    }
#define WAKEUP_CLEAR                                                                                                   \
    {                                                                                                                  \
        internalIO.digitalWrite(WAKEUP, LOW);                                                                          \
    }
#define PWRUP 4
#define PWRUP_SET                                                                                                      \
    {                                                                                                                  \
        internalIO.digitalWrite(PWRUP, HIGH);                                                                          \
    }
#define PWRUP_CLEAR                                                                                                    \
    {                                                                                                                  \
        internalIO.digitalWrite(PWRUP, LOW);                                                                           \
    }
#define VCOM 5
#define VCOM_SET                                                                                                       \
    {                                                                                                                  \
        internalIO.digitalWrite(VCOM, HIGH);                                                                           \
    }
#define VCOM_CLEAR                                                                                                     \
    {                                                                                                                  \
        internalIO.digitalWrite(VCOM, LOW);                                                                            \
    }
#define OE 0
#define OE_SET                                                                                                         \
    {                                                                                                                  \
        internalIO.digitalWrite(OE, HIGH);                                                                             \
    }
#define OE_CLEAR                                                                                                       \
    {                                                                                                                  \
        internalIO.digitalWrite(OE, LOW);                                                                              \
    }
#define GMOD 1
#define GMOD_SET                                                                                                       \
    {                                                                                                                  \
        internalIO.digitalWrite(GMOD, HIGH);                                                                           \
    }
#define GMOD_CLEAR                                                                                                     \
    {                                                                                                                  \
        internalIO.digitalWrite(GMOD, LOW);                                                                            \
    }
#define SPV 2
#define SPV_SET                                                                                                        \
    {                                                                                                                  \
        internalIO.digitalWrite(SPV, HIGH);                                                                            \
    }
#define SPV_CLEAR                                                                                                      \
    {                                                                                                                  \
        internalIO.digitalWrite(SPV, LOW);                                                                             \
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