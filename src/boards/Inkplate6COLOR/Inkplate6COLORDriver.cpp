// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATECOLOR
#include "Inkplate6COLORDriver.h"
#include "Inkplate-LVGL.h"

SPIClass spi2(2);
SdFat sd(&spi2);

SPIClass epdSPI(VSPI);
SPISettings epdSpiSettings(2000000, MSBFIRST, SPI_MODE0);

/**
 *
 * @brief       writePixelInternal funtion sets pixel data for (x, y) pixel position
 *
 * @param       int16_t x0
 *              default position for x, will be changed depending on rotation
 * @param       int16_t y0
 *              default position for y, will be changed depending on rotation
 * @param       uint16_t color
 *              pixel color, in 3bit mode have values in range 0-7
 *
 * @note        If x0 or y0 are out of inkplate screen borders, function will
 * exit.
 */
void EPDDriver::writePixelInternal(int16_t x, int16_t y, uint16_t color)
{
    int16_t x0 = x;
    int16_t y0 = y;
    if (x0 > E_INK_WIDTH - 1 || y0 > E_INK_HEIGHT - 1 || x0 < 0 || y0 < 0)
        return;
    if (color > 6)
        return;

    switch (_inkplate->getRotation())
    {
    case 3:
        _swap_int16_t(x0, y0);
        x0 = E_INK_HEIGHT - x0 - 1;
        break;
    case 0:
        x0 = E_INK_WIDTH - x0 - 1;
        y0 = E_INK_HEIGHT - y0 - 1;
        break;
    case 1:
        _swap_int16_t(x0, y0);
        y0 = E_INK_WIDTH - y0 - 1;
        break;
    }

    int _x = x0 / 2;
    int _x_sub = x0 % 2;
    uint8_t temp;
    temp = *(DMemory4Bit + E_INK_WIDTH / 2 * y0 + _x);
    *(DMemory4Bit + E_INK_WIDTH / 2 * y0 + _x) = (pixelMaskGLUT[_x_sub] & temp) | (_x_sub ? color : color << 4);
}


/**
 * @brief       display_flush_callback function is called whenever there is a change made on the current
 *              LVGL screen. The data is downscaled to a 3-bit color palette from RGB565
 *              and stored in the EPD buffer for rendering
 *
 * @param       lv_display_t *disp
 *              A pointer to the created LVGL display instance
 *
 * @param       lv_area_t *area
 *              A pointer to the area of the display which has changed
 *
 * @param       uint8_t px_map
 *              An array of pixel values in L8 format
 *
 */
void IRAM_ATTR display_flush_callback(lv_display_t *disp, const lv_area_t *area, uint8_t *px_map)
{
    Inkplate *self = (Inkplate *)lv_display_get_user_data(disp);

    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    // Validate input and boundaries
    if (w <= 0 || h <= 0 || px_map == NULL || area->x1 < 0 || area->y1 < 0 || area->x2 >= E_INK_WIDTH ||
        area->y2 >= E_INK_HEIGHT)
    {
        lv_display_flush_ready(disp);
        return;
    }
    if (self->ditherEnabled && self->_renderMode == LV_DISP_RENDER_MODE_FULL)
    {
        self->dither.ditherFramebuffer(px_map, E_INK_WIDTH, E_INK_HEIGHT);
    }
    else
    {
        // Framebuffer and constants
        uint8_t *buffer3b = self->DMemory4Bit;
        const int width_bytes_3b = E_INK_WIDTH / 2;
        const uint8_t *maskGLUT = pixelMaskGLUT;

        const uint8_t *src8 = px_map; // Source image in RGB565 (2 bytes per pixel)

        uint8_t R, G, B;

        for (int32_t y = 0; y < h; y++)
        {
            const uint8_t *src_row = src8 + (y * w * 2);

            for (int32_t x = 0; x < w; x++)
            {
                uint8_t lo = src_row[2 * x + 0];
                uint8_t hi = src_row[2 * x + 1];
                uint16_t pixel = (uint16_t)hi << 8 | lo;

                // Extract 5-6-5 bits and scale to 0–255 range
                uint8_t r5 = (pixel >> 11) & 0x1F;
                uint8_t g6 = (pixel >> 5) & 0x3F;
                uint8_t b5 = pixel & 0x1F;

                R = (r5 * 527 + 23) >> 6;
                G = (g6 * 259 + 33) >> 6;
                B = (b5 * 527 + 23) >> 6;

                // Convert to HSV
                float rf = R / 255.0f;
                float gf = G / 255.0f;
                float bf = B / 255.0f;

                float maxc = max(rf, max(gf, bf));
                float minc = min(rf, min(gf, bf));
                float delta = maxc - minc;

                float H = 0.0f; // hue 0–360
                float S = (maxc == 0) ? 0 : (delta / maxc);
                float V = maxc;

                // Compute hue
                if (delta > 0.0001f)
                {
                    if (maxc == rf)
                        H = 60.0f * fmod(((gf - bf) / delta), 6.0f);
                    else if (maxc == gf)
                        H = 60.0f * (((bf - rf) / delta) + 2.0f);
                    else
                        H = 60.0f * (((rf - gf) / delta) + 4.0f);
                }
                if (H < 0)
                    H += 360.0f;

                // Classification
                uint8_t color;

                if (S < 0.12f)
                {
                    if (V < 0.20f)
                        color = INKPLATE_BLACK;
                    else if (V > 0.85f)
                        color = INKPLATE_WHITE;
                    else
                        color = INKPLATE_YELLOW;
                }
                else
                {
                    if (H >= 190 && H < 260)
                        color = INKPLATE_BLUE;
                    else if (H >= 90 && H < 150)
                        color = INKPLATE_GREEN;
                    else if (H >= 15 && H < 45)
                        color = INKPLATE_ORANGE;
                    else if (H >= 45 && H < 90)
                        color = INKPLATE_YELLOW;
                    else
                        color = INKPLATE_RED;
                }


                // Apply 180° flip (Inkplate coordinate convention)
                int32_t sx = area->x1 + x;
                int32_t sy = area->y1 + y;
                int32_t fx = E_INK_WIDTH - sx - 1;
                int32_t fy = E_INK_HEIGHT - sy - 1;

                // Write pixel to 3-bit framebuffer (4-bit packed)
                int x_byte = fx / 2;
                int x_sub = fx % 2;
                uint8_t *dst_row = buffer3b + (width_bytes_3b * fy);

                uint8_t prev = dst_row[x_byte];
                uint8_t newv = (maskGLUT[x_sub] & prev) | (x_sub ? color : (color << 4));

                dst_row[x_byte] = newv;
            }
        }
    }

    lv_display_flush_ready(disp);
}

/**
 * @brief       begin function initialize Inkplate object with predefined
 * settings
 *
 * @param       uint8_t lightWaveform
 *              if inkplate doesn't work well or if it is fading after turning off
 *              lightWaveform should be set to 1 in order to fix that, but older boards
 *              may not support it
 *
 * @return      True if initialization is successful, false if failed or already
 * initialized
 */
int EPDDriver::initDriver(Inkplate *_inkplatePtr)
{
    unsigned long _timeout = 0;

    // Check if begin is called already. If it is, that means that panel needs to
    // be reseted in order to be woken up from deep sleep. If is this first time
    // begin has been called, init SPI, init pins, allocate memory for frame
    // buffer and clear frame buffer
    if (!_beginDone)
    {
        Wire.begin();

        // Save the given inkplate pointer for internal use
        _inkplate = _inkplatePtr;

        // Allocate memory for internal frame buffer
        DMemory4Bit = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 2);
        if (DMemory4Bit == NULL)
        {
            return false;
        }

        dither.begin(_paletteIdeal, _paletteIndex, paletteSize, _inkplatePtr);

        // Color whole frame buffer in white color
        memset(DMemory4Bit, INKPLATE_WHITE | (INKPLATE_WHITE << 4), E_INK_WIDTH * E_INK_HEIGHT / 2);

        _beginDone = true;
    }

    // Set IO expander to the output low state to reduce current consumption in sleep.
    setIOExpanderForLowPower();

    // Disable microSD card for the same reason.
    sdCardSleep();

    return true;
}


/**
 * @brief       clearDisplay function clears memory buffer for display
 *
 * @note        This does not clear the actual display, only the memory buffer, you need to call
 * display() function after this to clear the display
 */
void EPDDriver::clearDisplay()
{
    memset(DMemory4Bit, WHITE << 4 | WHITE, E_INK_WIDTH * E_INK_HEIGHT / 2);
}

/**
 * @brief       display function update display with new data from buffer
 *
 * @param       bool leaveOn
 *              if set to 1, it will disable turning supply for eink after
 *              display update in order to save some time needed for power supply
 *              to save some time at next display update or increase refreshing speed
 */
void EPDDriver::display(bool _leaveOn)
{
    // Wake the panel back up
    setPanelDeepSleep(false);

    // Set resolution setting
    uint8_t res_set_data[] = {0x02, 0x58, 0x01, 0xc0};
    sendCommand(0x61);
    sendData(res_set_data, 4);

    // Push pixel data to the epaper RAM
    sendCommand(0x10);
    digitalWrite(EPAPER_DC_PIN, HIGH);
    digitalWrite(EPAPER_CS_PIN, LOW);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.writeBytes(DMemory4Bit, E_INK_WIDTH * E_INK_HEIGHT / 2);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);

    sendCommand(POWER_OFF_REGISTER);
    while (!(digitalRead(EPAPER_BUSY_PIN)))
        ; // Wait for busy high signal
    sendCommand(DISPLAY_REF_REGISTER);
    while (!(digitalRead(EPAPER_BUSY_PIN)))
        ; // Wait for busy high signal
    sendCommand(0x02);
    while (digitalRead(EPAPER_BUSY_PIN))
        ; // Wait for busy low signal
    delay(200);

    // Put the panel to sleep again
    setPanelDeepSleep(true);
}


uint8_t EPDDriver::getPanelState()
{
    return _panelState;
}
void EPDDriver::setPanelState(uint8_t state)
{
    _panelState = state;
}

/**
 * @brief       clean function cleans screen of any potential burn in
 *
 *              Based on c param it will: if c=0 light screen, c=1 darken the
 * screen, c=2 discharge the screen or 3 skip all pixels
 *
 * @param       uint8_t c
 *              one of four posible pixel states
 *
 * @param       uint8_t rep
 *              Number of repetitions
 *
 *
 * @note        Should not be used in intervals smaller than 5 seconds
 */
void EPDDriver::clean()
{

    // Set resolution setting
    uint8_t res_set_data[] = {0x02, 0x58, 0x01, 0xc0};
    sendCommand(0x61);
    sendData(res_set_data, 4);

    // Push pixel data to the epaper RAM
    sendCommand(0x10);
    digitalWrite(EPAPER_DC_PIN, HIGH);
    digitalWrite(EPAPER_CS_PIN, LOW);
    epdSPI.beginTransaction(epdSpiSettings);
    for (uint32_t i = 0; i < (E_INK_WIDTH * E_INK_HEIGHT / 2); i++)
    {
        epdSPI.transfer(INKPLATE_WHITE | INKPLATE_WHITE << 4);
    }
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);

    sendCommand(POWER_OFF_REGISTER);
    while (!(digitalRead(EPAPER_BUSY_PIN)))
        ; // Wait for busy high signal
    sendCommand(DISPLAY_REF_REGISTER);
    while (!(digitalRead(EPAPER_BUSY_PIN)))
        ; // Wait for busy high signal
    sendCommand(POWER_OFF_REGISTER);
    while (digitalRead(EPAPER_BUSY_PIN))
        ; // Wait for busy low signal
    delay(200);
}

/**
 * @brief       resetPanel resets Inkplate 6COLOR
 */
void EPDDriver::resetPanel()
{
    digitalWrite(EPAPER_RST_PIN, LOW);
    delay(1);
    digitalWrite(EPAPER_RST_PIN, HIGH);
    delay(200);
}

/**
 * @brief       sendCommand sends SPI command to Inkplate 6COLOR
 *
 * @param       uint8_t _command
 *              predefined command for epaper control
 */
void EPDDriver::sendCommand(uint8_t _command)
{
    digitalWrite(EPAPER_DC_PIN, LOW);
    digitalWrite(EPAPER_CS_PIN, LOW);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.transfer(_command);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);
}

/**
 * @brief       sendData sends SPI data to Inkplate 6COLOR
 *
 * @param       uint8_t *_data
 *              pointer to data buffer to be sent to epaper
 * @param       int _n
 *              number of data bytes
 */
void EPDDriver::sendData(uint8_t *_data, int _n)
{
    digitalWrite(EPAPER_DC_PIN, HIGH);
    digitalWrite(EPAPER_CS_PIN, LOW);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.writeBytes(_data, _n);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);
}

/**
 * @brief       sendData sends SPI data to Inkplate 6COLOR
 *
 * @param       uint8_t _data
 *              data buffer to be sent to epaper
 */
void EPDDriver::sendData(uint8_t _data)
{
    digitalWrite(EPAPER_DC_PIN, HIGH);
    digitalWrite(EPAPER_CS_PIN, LOW);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.transfer(_data);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);
}

/**
 * @brief       setPanelDeepSleep puts the color ePaper into deep sleep, or wakes it and reinitializes it
 *
 * @param       bool _state
 *              -'True' sets the panel to sleep
 *              -'False' wakes the panel
 *
 * @returns     True if successful, False if unsuccessful
 *
 */
bool EPDDriver::setPanelDeepSleep(bool _state)
{
    if (!_state)
    {
        // _state is false? Wake the panel!

        // Send commands to power up panel. According to the datasheet, it can be
        // powered up from deep sleep only by reseting it and doing reinit.

        // Init SPI
        epdSPI.begin(EPAPER_CLK, -1, EPAPER_DIN, -1);
        pinMode(EPAPER_BUSY_PIN, INPUT);
        pinMode(EPAPER_RST_PIN, OUTPUT);
        pinMode(EPAPER_DC_PIN, OUTPUT);
        pinMode(EPAPER_CS_PIN, OUTPUT);

        // De-select epaper (and also try to charge capacitors to avoid big in-rush current).
        digitalWrite(EPAPER_DC_PIN, HIGH);
        digitalWrite(EPAPER_CS_PIN, HIGH);

        // Wait a little bit.
        delay(100);

        // Reset epaper
        resetPanel();

        // Wait for ePaper to be ready by reading busy high signal
        float _timeout = millis();
        while ((!digitalRead(EPAPER_BUSY_PIN)) && ((millis() - _timeout) < INIT_TIMEOUT))
            ;

        if (!digitalRead(EPAPER_BUSY_PIN))
            return false;

        // Send whole bunch of commands and data
        uint8_t panel_set_data[] = {0xEF, 0x08};
        sendCommand(PANEL_SET_REGISTER);
        sendData(panel_set_data, 2);

        uint8_t power_set_data[] = {0x37, 0x00, 0x05, 0x05};
        sendCommand(POWER_SET_REGISTER);
        sendData(power_set_data, 4);

        sendCommand(POWER_OFF_SEQ_SET_REGISTER);
        sendData(0x00);

        uint8_t booster_softstart_data[] = {0xC7, 0xC7, 0x1D};
        sendCommand(BOOSTER_SOFTSTART_REGISTER);
        sendData(booster_softstart_data, 3);

        sendCommand(TEMP_SENSOR_EN_REGISTER);
        sendData(0x00);

        sendCommand(VCOM_DATA_INTERVAL_REGISTER);
        sendData(0x37);

        sendCommand(0x60);
        sendData(0x20);

        uint8_t res_set_data[] = {0x02, 0x58, 0x01, 0xC0};
        sendCommand(RESOLUTION_SET_REGISTER);
        sendData(res_set_data, 4);

        sendCommand(0xE3);
        sendData(0xAA);

        delay(100);
        sendCommand(VCOM_DATA_INTERVAL_REGISTER);
        sendData(0x37);

        return true;
    }
    else
    {
        // _state is true? Put the panel to sleep.

        delay(10);
        sendCommand(DEEP_SLEEP_REGISTER);
        sendData(0xA5);
        delay(100);
        digitalWrite(EPAPER_RST_PIN, LOW);
        delay(100);
        digitalWrite(EPAPER_DC_PIN, LOW);
        digitalWrite(EPAPER_CS_PIN, LOW);
        epdSPI.end();

        pinMode(EPAPER_CLK, OUTPUT);
        pinMode(EPAPER_DIN, OUTPUT);
        digitalWrite(EPAPER_CLK, LOW);
        digitalWrite(EPAPER_DIN, LOW);

        return true;
    }
}

/**
 * @brief       setIOExpanderForLowPower initiates I/O Expander pins for low power, and puts
 * them in OUTPUT LOW because they are using least amount of current in deep
 * sleep that way
 */
void EPDDriver::setIOExpanderForLowPower()
{
    Wire.begin();
    externalIO.begin(IO_INT_ADDR);

    // TOUCHPAD PINS
    externalIO.pinMode(IO_PIN_B2, OUTPUT);
    externalIO.pinMode(IO_PIN_B3, OUTPUT);
    externalIO.pinMode(IO_PIN_B4, OUTPUT);

    externalIO.digitalWrite(IO_PIN_B2, LOW);
    externalIO.digitalWrite(IO_PIN_B3, LOW);
    externalIO.digitalWrite(IO_PIN_B4, LOW);

    // Battery voltage Switch MOSFET
    externalIO.pinMode(IO_PIN_B1, OUTPUT);
    externalIO.digitalWrite(IO_PIN_B1, LOW);

    // Rest of pins go to OUTPUT LOW state because in deepSleep mode they are
    // using least amount of power
    externalIO.pinMode(IO_PIN_A0, OUTPUT);
    externalIO.pinMode(IO_PIN_A1, OUTPUT);
    externalIO.pinMode(IO_PIN_A2, OUTPUT);
    externalIO.pinMode(IO_PIN_A3, OUTPUT);
    externalIO.pinMode(IO_PIN_A4, OUTPUT);
    externalIO.pinMode(IO_PIN_A5, OUTPUT);
    externalIO.pinMode(IO_PIN_A6, OUTPUT);
    externalIO.pinMode(IO_PIN_A7, OUTPUT);
    externalIO.pinMode(IO_PIN_B0, OUTPUT);
    externalIO.pinMode(IO_PIN_B5, OUTPUT);
    externalIO.pinMode(IO_PIN_B6, OUTPUT);
    externalIO.pinMode(IO_PIN_B7, OUTPUT);

    externalIO.digitalWrite(IO_PIN_A0, LOW);
    externalIO.digitalWrite(IO_PIN_A1, LOW);
    externalIO.digitalWrite(IO_PIN_A2, LOW);
    externalIO.digitalWrite(IO_PIN_A3, LOW);
    externalIO.digitalWrite(IO_PIN_A4, LOW);
    externalIO.digitalWrite(IO_PIN_A5, LOW);
    externalIO.digitalWrite(IO_PIN_A6, LOW);
    externalIO.digitalWrite(IO_PIN_A7, LOW);
    externalIO.digitalWrite(IO_PIN_B0, LOW);
    externalIO.digitalWrite(IO_PIN_B5, LOW);
    externalIO.digitalWrite(IO_PIN_B6, LOW);
    externalIO.digitalWrite(IO_PIN_B7, LOW);
}


/**
 * @brief       sdCardInit initializes sd card trough SPI
 *
 * @return      0 if failed to initialise, 1 if successful
 */
int16_t EPDDriver::sdCardInit()
{
    externalIO.pinMode(SD_PMOS_PIN, OUTPUT);
    externalIO.digitalWrite(SD_PMOS_PIN, LOW);
    delay(50);
    spi2.begin(14, 12, 13, 15);
    setSdCardOk(sd.begin(15, SD_SCK_MHZ(25)));
    return getSdCardOk();
}

/**
 * @brief       sdCardSleep turns off the P-MOS which powers the sd card to save energy in deep sleep
 */
void EPDDriver::sdCardSleep()
{
    // Set SPI pins to input to reduce power consumption in deep sleep
    pinMode(12, INPUT);
    pinMode(13, INPUT);
    pinMode(14, INPUT);
    pinMode(15, INPUT);

    // And also disable uSD card supply
    externalIO.pinMode(SD_PMOS_PIN, INPUT);
}

/**
 * @brief       getSdFat gets sd card object
 *
 * @return      sd card class object
 */
SdFat EPDDriver::getSdFat()
{
    return sd;
}

/**
 * @brief       getSPIptr gets SPI class object pointer
 *
 * @return      SPI class object
 */
SPIClass *EPDDriver::getSPIptr()
{
    return &spi2;
}

/**
 * @brief       setSdCardOk sets sd card OK status
 *
 * @param       int16_t s
 *              sd card OK status, can be 1 or 0
 */
void EPDDriver::setSdCardOk(int16_t s)
{
    _sdCardOk = s;
}


/**
 * @brief       setSdCardOk gets sd card OK status
 *
 * @return      sd card OK status, can be 1 or 0
 */
int16_t EPDDriver::getSdCardOk()
{
    return _sdCardOk;
}


/**
 * @brief       readBattery reads voltage of the battery
 *
 * @return      returns battery voltage value
 */
double EPDDriver::readBattery()
{
    // Read the pin on the battery MOSFET. If is high, that means is older version of the board
    // that uses PMOS only. If it's low, newer board with both PMOS and NMOS.
    externalIO.pinMode(9, INPUT);
    int state = externalIO.digitalRead(9);
    externalIO.pinMode(9, OUTPUT);

    // If the input is pulled high, it's PMOS only.
    // If it's pulled low, it's PMOS and NMOS.
    if (state)
    {
        externalIO.digitalWrite(9, LOW);
    }
    else
    {
        externalIO.digitalWrite(9, HIGH);
    }

    // Wait a little bit after a MOSFET enable.
    delay(5);

    // Set to the highest resolution and read the voltage.
    analogReadResolution(12);
    int adc = analogReadMilliVolts(35);

    // Turn off the MOSFET (and voltage divider).
    if (state)
    {
        externalIO.digitalWrite(9, HIGH);
    }
    else
    {
        externalIO.digitalWrite(9, LOW);
    }

    // Calculate the voltage at the battery terminal (voltage is divided in half by voltage divider).
    return (double(adc) * 2.0 / 1000);
}


#endif