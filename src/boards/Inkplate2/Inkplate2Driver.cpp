// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE2
#include "Inkplate2Driver.h"
#include "Inkplate-LVGL.h"

SPIClass epdSPI(VSPI);

SPISettings epdSpiSettings(1000000UL, MSBFIRST, SPI_MODE0);

void EPDDriver::writePixelInternal(int16_t x0, int16_t y0, uint16_t color)
{
    if (x0 > E_INK_HEIGHT - 1 || y0 > E_INK_WIDTH  - 1 || x0 < 0 || y0 < 0)
        return;
    if (color > 2)
        return;
    _swap_int16_t(x0, y0);
    y0 = E_INK_HEIGHT - y0 - 1;

    // Find the specific byte in the frame buffer that needs to be modified.
    // Also find the bit in the byte that needs modification.
    int _x = x0 / 8;
    int _xSub = x0 % 8;

    int _position = E_INK_WIDTH/ 8 * y0 + _x;

    // Clear both black and red frame buffer.
    *(DMemory4Bit + _position) |= (pixelMaskLUT[7 - _xSub]);
    *(DMemory4Bit + (E_INK_WIDTH * E_INK_HEIGHT / 8) + _position) |= (pixelMaskLUT[7 - _xSub]);

    // To optimize writing pixels into EPD, framebuffer is split in half, where first half is for B&W pixels and other
    // half is for red pixels only
    if (color < 2)
    {
        *(DMemory4Bit + _position) &= ~(color << (7 - _xSub));
    }
    else
    {
        *(DMemory4Bit + (E_INK_WIDTH * E_INK_HEIGHT / 8) + _position) &= ~(pixelMaskLUT[7 - _xSub]);
    }
}




/**
 * @brief       display_flush_callback function is called whenever there is a change made on the current 
 *              LVGL screen. The data is downscaled to a White-Black-Red color palette from RGB565
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

    if(self->ditherEnabled)
    {
        self->dither.ditherFramebuffer(px_map, E_INK_HEIGHT, E_INK_WIDTH);
    }
    else
    {
        const uint8_t *src8 = px_map;

        for (int32_t y = 0; y < h; y++)
        {
            const uint8_t *src_row = src8 + (y * w * 2); // 2 bytes per pixel

            for (int32_t x = 0; x < w; x++)
            {
                // Read 16-bit pixel (RGB565)
                uint16_t pixel = src_row[x * 2] | (src_row[x * 2 + 1] << 8);

                // Extract RGB components
                uint8_t r = (pixel >> 11) & 0x1F;
                uint8_t g = (pixel >> 5) & 0x3F;
                uint8_t b = pixel & 0x1F;

                // Scale up to 8-bit per channel
                r = (r * 255) / 31;
                g = (g * 255) / 63;
                b = (b * 255) / 31;

                // Convert to brightness
                uint8_t gray = (uint8_t)((r * 0.299f) + (g * 0.587f) + (b * 0.114f));

                uint16_t color;
                if (gray < 85)
                    color = 1; // Black
                else if (gray > 180)
                    color = 0; // Red
                else
                    color = 2; // White

                self->writePixelInternal(area->x1 + x, area->y1 + y, color);
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
    if (!_beginDone)
    {
        // Allocate memory for frame buffer
        DMemory4Bit = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 4);

        _inkplate = _inkplatePtr;

        dither.begin(_paletteIdeal, _paletteIndex, paletteSize, _inkplatePtr);

        if (DMemory4Bit == NULL)
        {
            return false;
        }

        // Clear frame buffer
        clearDisplay();

        // Set default rotation
        _inkplate->setRotation(1);

        _beginDone = 1;
    }

    // Wake the ePaper and initialize everything
    // If it fails, return false
    if (!setPanelDeepSleep(false))
        return false;

    // Put the panel to deep sleep
    // The panel is always in sleep unless it's being written display data to
    setPanelDeepSleep(true);
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
    memset(DMemory4Bit, 0xFF, E_INK_WIDTH * E_INK_HEIGHT / 4);
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
       // Wake the panel and wait a bit
    // The refresh time is long anyway so this delay doesn't make much impact
    setPanelDeepSleep(false);
    delay(20);

    // First write B&W pixels to epaper
    sendCommand(0x10);
    sendData(DMemory4Bit, (E_INK_WIDTH * E_INK_HEIGHT / 8));

    // Now write red pixels to epaper
    sendCommand(0x13);
    sendData(DMemory4Bit + (E_INK_WIDTH * E_INK_HEIGHT / 8), (E_INK_WIDTH * E_INK_HEIGHT / 8));

    // Stop data transfer
    sendCommand(0x11);
    sendData(0x00);

    // Send display refresh command
    sendCommand(0x12);
    delayMicroseconds(500); // Wait at least 200 uS
    waitForEpd(60000);

    // Go back to sleep
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
 * @brief       resetPanel resets Inkplate 6COLOR
 */
void EPDDriver::resetPanel()
{
    digitalWrite(EPAPER_RST_PIN, LOW);
    delay(100);
    digitalWrite(EPAPER_RST_PIN, HIGH);
    delay(100);
}

/**
 * @brief       sendCommand sends SPI command to Inkplate 6COLOR
 *
 * @param       uint8_t _command
 *              predefined command for epaper control
 */
void EPDDriver::sendCommand(uint8_t _command)
{
     digitalWrite(EPAPER_CS_PIN, LOW);
    digitalWrite(EPAPER_DC_PIN, LOW);
    delayMicroseconds(10);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.transfer(_command);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);
    delay(1);
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
    digitalWrite(EPAPER_CS_PIN, LOW);
    digitalWrite(EPAPER_DC_PIN, HIGH);
    delayMicroseconds(10);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.writeBytes(_data, _n);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);
    delay(1);
}

/**
 * @brief       sendData sends SPI data to Inkplate 6COLOR
 *
 * @param       uint8_t _data
 *              data buffer to be sent to epaper
 */
void EPDDriver::sendData(uint8_t _data)
{
    digitalWrite(EPAPER_CS_PIN, LOW);
    digitalWrite(EPAPER_DC_PIN, HIGH);
    delayMicroseconds(10);
    epdSPI.beginTransaction(epdSpiSettings);
    epdSPI.transfer(_data);
    epdSPI.endTransaction();
    digitalWrite(EPAPER_CS_PIN, HIGH);
    delay(1);
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

        // Set SPI pins
        epdSPI.begin(EPAPER_CLK, -1, EPAPER_DIN, -1);

        // Set up EPD communication pins
        pinMode(EPAPER_CS_PIN, OUTPUT);
        pinMode(EPAPER_DC_PIN, OUTPUT);
        pinMode(EPAPER_RST_PIN, OUTPUT);
        pinMode(EPAPER_BUSY_PIN, INPUT_PULLUP);

        delay(10);

        // Reinit the panel
        // Reset EPD IC
        resetPanel();

        sendCommand(0x04);
        if (!waitForEpd(BUSY_TIMEOUT_MS))
            return false; // Waiting for the electronic paper IC to release the idle signal

        sendCommand(0x00); // Enter panel setting
        sendData(0x0f);    // LUT from OTP 128x296
        sendData(0x89);    // Temperature sensor, boost and other related timing settings

        sendCommand(0x61); // Enter panel resolution setting
        sendData(E_INK_WIDTH);
        sendData(E_INK_HEIGHT >> 8);
        sendData(E_INK_HEIGHT & 0xff);

        sendCommand(0x50); // VCOM and data interval setting
        sendData(0x77);    // WBmode:VBDF 17|D7 VBDW 97 VBDB 57   WBRmode:VBDF F7 VBDW 77 VBDB 37  VBDR B7

        return true;
    }
    else
    {
        // _state is true? Put the panel to sleep.

        sendCommand(0X50); // VCOM and data interval setting
        sendData(0xf7);
        sendCommand(0X02); // Power  EPD off
        waitForEpd(BUSY_TIMEOUT_MS);
        sendCommand(0X07); // Put EPD in deep sleep
        sendData(0xA5);
        delay(1);

        // Disable SPI
        epdSPI.end();

        // To reduce power consumption, set SPI pins as outputs
        pinMode(EPAPER_RST_PIN, INPUT);
        pinMode(EPAPER_DC_PIN, INPUT);
        pinMode(EPAPER_CS_PIN, INPUT);
        pinMode(EPAPER_BUSY_PIN, INPUT);
        pinMode(EPAPER_CLK, INPUT);
        pinMode(EPAPER_DIN, INPUT);

        return true;
    }
}


/**
 * @brief       Waits for panel to be ready for data
 *
 * @param       uint8_t _timeout
 *              Timeout for wait
 *
 * @return      bool is panel ready or timed out
 */
bool EPDDriver::waitForEpd(uint16_t _timeout)
{
    unsigned long _time = millis();
    while (!digitalRead(EPAPER_BUSY_PIN) && ((millis() - _time) < _timeout))
        ;
    if (!digitalRead(EPAPER_BUSY_PIN))
        return false;
    delay(200);
    return true;
}


#endif