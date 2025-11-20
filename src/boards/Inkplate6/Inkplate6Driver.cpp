
#ifdef ARDUINO_INKPLATE6V2
#include "Inkplate6Driver.h"
#include "Inkplate-LVGL.h"

SPIClass spi2(2);
SdFat sd(&spi2);

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

    // set x, y depending on selected rotation
    switch (_inkplate->getRotation())
    {
    case 1: // 90 degree left
        _swap_int16_t(x0, y0);
        x0 = E_INK_HEIGHT - x0 - 1;
        break;
    case 2: // 180 degree, or upside down
        x0 = E_INK_WIDTH - x0 - 1;
        y0 = E_INK_HEIGHT - y0 - 1;
        break;
    case 3: // 90 degree right
        _swap_int16_t(x0, y0);
        y0 = E_INK_WIDTH - y0 - 1;
        break;
    }

    // If the 1 bit mode is used, pixels are packed 1 bit = 1 pixel in frame buffer
    if (getDisplayMode() == 0)
    {
        // Divide by 8 to find a byte.
        int x = x0 >> 3;

        // Get the remainder of the division to find a exact bit in the byte that needs to be modified.
        int x_sub = x0 & 7;

        // Save the currnet state of the byte in the frame buffer.
        uint8_t temp = *(_partial + (E_INK_WIDTH / 8) * y0 + x);

        // Modify the pixel. First clear the pixel by writing zero then write the 1 if the pixel is set.
        *(_partial + (E_INK_WIDTH / 8) * y0 + x) = (~pixelMaskLUT[x_sub] & temp) | (color ? pixelMaskLUT[x_sub] : 0);
    }
    else
    {
        // If 3 bit mode is used, constrain the color value (only 8 possible colors are available).
        color &= 7;

        // Divide by two to find a byte
        int x = x0 >> 1;

        //  Get the remainder of the division to find if the lower or upper 4 bits are needed.
        int x_sub = x0 & 1;

        // Store the current value of the byte.
        uint8_t temp;
        temp = *(DMemory4Bit + (E_INK_WIDTH / 2) * y0 + x);

        // Modify the specific pixel by writing all zeros into lower or upper 4 bits and set the needed color.
        *(DMemory4Bit + (E_INK_WIDTH / 2) * y0 + x) = (pixelMaskGLUT[x_sub] & temp) | (x_sub ? color : color << 4);
    }
}


/**
 * @brief       display_flush_callback function is called whenever there is a change made on the current
 *              LVGL screen. The data is downscaled to 3 bit or 1 bit grayscale depending on the current display mode
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
    Inkplate *self = static_cast<Inkplate *>(lv_display_get_user_data(disp));

    int32_t w = lv_area_get_width(area);
    int32_t h = lv_area_get_height(area);

    if (w <= 0 || h <= 0 || px_map == nullptr || area->x1 < 0 || area->y1 < 0 || area->x2 >= E_INK_WIDTH ||
        area->y2 >= E_INK_HEIGHT)
    {
        lv_display_flush_ready(disp);
        return;
    }

    bool is3bit = (self->getDisplayMode() == INKPLATE_3BIT);

    if (self->ditherEnabled && self->_renderMode == LV_DISP_RENDER_MODE_FULL)
    {
        self->dither.ditherFramebuffer(px_map, E_INK_WIDTH, E_INK_HEIGHT, is3bit);
    }
    else
    {
        uint8_t *buffer1b = self->_partial;
        uint8_t *buffer3b = self->DMemory4Bit;

        const int width_bytes_1b = E_INK_WIDTH / 8;
        const int width_bytes_3b = E_INK_WIDTH / 2;

        // Preload LUTs to local for faster access
        const uint8_t *maskLUT = pixelMaskLUT;
        const uint8_t *maskGLUT = pixelMaskGLUT;

        for (int32_t y = 0; y < h; y++)
        {
            int32_t screen_y = area->y1 + y;
            const uint8_t *src_row = px_map + (y * w);

            if (is3bit)
            {
                uint8_t *dst_row = buffer3b + (width_bytes_3b * screen_y);

                for (int32_t x = 0; x < w; x++)
                {
                    int32_t screen_x = area->x1 + x;
                    if (screen_x >= E_INK_WIDTH)
                        break;

                    uint8_t gray3 = src_row[x] >> 5;
                    int x_byte = screen_x / 2;
                    int x_sub = screen_x % 2;

                    uint8_t temp = dst_row[x_byte];
                    uint8_t newv = (maskGLUT[x_sub] & temp) | (x_sub ? gray3 : (gray3 << 4));

                    dst_row[x_byte] = newv;
                }
            }
            else
            {
                uint8_t *dst_row = buffer1b + (width_bytes_1b * screen_y);

                for (int32_t x = 0; x < w; x++)
                {
                    int32_t screen_x = area->x1 + x;
                    if (screen_x >= E_INK_WIDTH)
                        break;

                    uint8_t gray = src_row[x];
                    uint8_t bit = (gray < 128) ? 1 : 0;

                    int x_byte = screen_x / 8;
                    int x_sub = screen_x % 8;

                    uint8_t temp = dst_row[x_byte];
                    // Preserve other bits using original mask logic
                    dst_row[x_byte] = (~maskLUT[x_sub] & temp) | (bit ? maskLUT[x_sub] : 0);
                }
            }
        }
    }

    lv_display_flush_ready(disp);
}


/**
 * @brief       begin function initialize Inkplate object with predefined
 * settings
 *
 * @param       Inkplate _inkplatePtr
 *              A pointer to the created Inkplate instance which will be used by subclasses
 *              to use internal inkplate functions
 *
 * @return      True if initialization is successful, false if failed or already
 * initialized
 */
int EPDDriver::initDriver(Inkplate *_inkplatePtr)
{
    // If the driver was already initialized, skip current initialization
    if (_beginDone == 1)
        return 0;

    Wire.begin();

    // Save the given inkplate pointer for internal use
    _inkplate = _inkplatePtr;

    // Initialize the GPIOs
    gpioInit();


    // Use only myI2S
    myI2S = &I2S1;

    // Allocate memory for DMA descriptor and line buffer.
    _dmaLineBuffer = (uint8_t *)heap_caps_malloc((E_INK_WIDTH / 4) + 16, MALLOC_CAP_DMA);
    _dmaI2SDesc = (lldesc_s *)heap_caps_malloc(sizeof(lldesc_t), MALLOC_CAP_DMA);

    if (_dmaLineBuffer == NULL || _dmaI2SDesc == NULL)
    {
        return 0;
    }

    dither.begin(_inkplatePtr);

    // Init the I2S driver. It will setup a I2S driver.
    I2SInit(myI2S);

    // CONTROL PINS
    pinMode(0, OUTPUT);
    pinMode(2, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);
    internalIO.pinMode(OE, OUTPUT);
    internalIO.pinMode(GMOD, OUTPUT);
    internalIO.pinMode(SPV, OUTPUT);

    internalIO.pinMode(10, OUTPUT);
    internalIO.pinMode(11, OUTPUT);
    internalIO.pinMode(12, OUTPUT);
    internalIO.digitalWrite(10, LOW);
    internalIO.digitalWrite(11, LOW);
    internalIO.digitalWrite(12, LOW);
    // Battery voltage Switch MOSFET
    internalIO.pinMode(9, OUTPUT);
    internalIO.digitalWrite(9, LOW);


    if (!initializeFramebuffers())
    {
        return 0;
    }

    // Calculate color LUTs to optimize drawing to the screen
    calculateLUTs();
    _beginDone = 1;
    return 1;
}


/**
 * @brief       Calculates the values of the lookup table to
 *              speed up rendering
 */
void EPDDriver::calculateLUTs()
{
    for (int j = 0; j < 9; ++j)
    {
        for (int i = 0; i < 256; ++i)
        {
            GLUT[j * 256 + i] = (waveform3Bit[i & 0x07][j] << 2) | (waveform3Bit[(i >> 4) & 0x07][j]);
            GLUT2[j * 256 + i] = ((waveform3Bit[i & 0x07][j] << 2) | (waveform3Bit[(i >> 4) & 0x07][j])) << 4;
        }
    }
}


/**
 * @brief       vscan_start starts writing new frame and skips first two lines
 * that are invisible on screen
 */
void EPDDriver::vscan_start()
{
    CKV_SET;
    delayMicroseconds(7);
    SPV_CLEAR;
    delayMicroseconds(10);
    CKV_CLEAR;
    delayMicroseconds(0);
    CKV_SET;
    delayMicroseconds(8);
    SPV_SET;
    delayMicroseconds(10);
    CKV_CLEAR;
    delayMicroseconds(0);
    CKV_SET;
    delayMicroseconds(18);
    CKV_CLEAR;
    delayMicroseconds(0);
    CKV_SET;
    delayMicroseconds(18);
    CKV_CLEAR;
    delayMicroseconds(0);
    CKV_SET;
}

/**
 * @brief       vscan_end ends current row and prints data to screen
 */
void EPDDriver::vscan_end()
{
    CKV_CLEAR;
    LE_SET;
    LE_CLEAR;
    delayMicroseconds(0);
}

/**
 * @brief       sets the current display mode of the e-ink display
 *
 * @param       uint8_t
 *              if set to 1, it will be set to grayscale mode
 *              if set to 0, set to BW mode
 */
void EPDDriver::selectDisplayMode(uint8_t displayMode)
{
    _displayMode = displayMode;
}

/**
 * @brief       clearDisplay function clears memory buffer for display
 *
 * @note        This does not clear the actual display, only the memory buffer, you need to call
 * display() function after this to clear the display
 */
void EPDDriver::clearDisplay()
{
    // Clear 1 bit per pixel display buffer
    if (_displayMode == 0)
        memset(_partial, 0, E_INK_WIDTH * E_INK_HEIGHT / 8);

    // Clear 3 bit per pixel display buffer
    else if (_displayMode == 1)
        memset(DMemory4Bit, 255, E_INK_WIDTH * E_INK_HEIGHT / 2);
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
    if (_displayMode == 0)
    {
        display1b(_leaveOn);
    }
    else if (_displayMode == 1)
    {
        display3b(_leaveOn);
    }
}

/**
 * @brief       display3b function writes grayscale data to display
 *
 * @param       bool leaveOn
 *              if set to 1, it will disable turning supply for eink after
 *              display update in order to save some time needed for power supply
 *              to save some time at next display update or increase refreshing speed
 */
void EPDDriver::display3b(bool leaveOn)
{
    if (!einkOn())
        return;

    clean(0, 1);
    clean(1, 18);
    clean(2, 1);
    clean(0, 18);
    clean(2, 1);
    clean(1, 18);
    clean(2, 1);
    clean(0, 18);
    clean(2, 1);

    for (int k = 0; k < 9; ++k)
    {
        uint8_t *dp = DMemory4Bit + E_INK_WIDTH * E_INK_HEIGHT / 2;

        vscan_start();
        for (int i = 0; i < E_INK_HEIGHT; ++i)
        {
            for (int j = 0; j < (E_INK_WIDTH / 4); j += 4)
            {
                _dmaLineBuffer[j + 2] = (GLUT2[k * 256 + (*(--dp))] | GLUT[k * 256 + (*(--dp))]);
                _dmaLineBuffer[j + 3] = (GLUT2[k * 256 + (*(--dp))] | GLUT[k * 256 + (*(--dp))]);
                _dmaLineBuffer[j] = (GLUT2[k * 256 + (*(--dp))] | GLUT[k * 256 + (*(--dp))]);
                _dmaLineBuffer[j + 1] = (GLUT2[k * 256 + (*(--dp))] | GLUT[k * 256 + (*(--dp))]);
            }
            sendDataI2S(myI2S, _dmaI2SDesc);
            vscan_end();
        }
        delayMicroseconds(230);
    }
    clean(3, 1);
    vscan_start();

    if (!leaveOn)
        einkOff();
}

/**
 *
 * @brief       display1b function writes black and white data to display
 *
 * @param       bool leaveOn
 *              if set to 1, it will disable turning supply for eink after
 *              display update in order to save some time needed for power supply
 *              to save some time at next display update or increase refreshing speed
 */
void EPDDriver::display1b(bool leaveOn)
{
    memcpy(DMemoryNew, _partial, E_INK_WIDTH * E_INK_HEIGHT / 8);

    uint32_t _send;
    uint8_t data;
    uint8_t dram;

    if (!einkOn())
        return;

    clean(0, 1);
    clean(1, 18);
    clean(2, 1);
    clean(0, 18);
    clean(2, 1);
    clean(1, 18);
    clean(2, 1);
    clean(0, 18);
    clean(2, 1);

// How many cycles / phases / frames of dark pixels
#if defined(ARDUINO_ESP32_DEV)
    int rep = 4;
#elif defined(ARDUINO_INKPLATE6V2)
    int rep = 5;
#endif

    for (int k = 0; k < rep; ++k)
    {
        uint8_t *DMemoryNewPtr = DMemoryNew + (E_INK_WIDTH * E_INK_HEIGHT / 8) - 1;
        vscan_start();
        for (int i = 0; i < E_INK_HEIGHT; ++i)
        {
            for (int n = 0; n < (E_INK_WIDTH / 4); n += 4)
            {
                uint8_t dram1 = *(DMemoryNewPtr);
                uint8_t dram2 = *(DMemoryNewPtr - 1);
                _dmaLineBuffer[n] = LUTB[(dram2 >> 4) & 0x0F];     // i + 2;
                _dmaLineBuffer[n + 1] = LUTB[dram2 & 0x0F];        // i + 3;
                _dmaLineBuffer[n + 2] = LUTB[(dram1 >> 4) & 0x0F]; // i;
                _dmaLineBuffer[n + 3] = LUTB[dram1 & 0x0F];        // i + 1;
                DMemoryNewPtr -= 2;
            }
            // Send the data using I2S DMA driver.
            sendDataI2S(myI2S, _dmaI2SDesc);
            vscan_end();
        }
        delayMicroseconds(230);
    }

    for (int k = 0; k < 1; ++k)
    {
        uint8_t *DMemoryNewPtr = DMemoryNew + (E_INK_WIDTH * E_INK_HEIGHT / 8) - 1;
        vscan_start();
        for (int i = 0; i < E_INK_HEIGHT; ++i)
        {
            for (int n = 0; n < (E_INK_WIDTH / 4); n += 4)
            {
                uint8_t dram1 = *(DMemoryNewPtr);
                uint8_t dram2 = *(DMemoryNewPtr - 1);
                _dmaLineBuffer[n] = LUT2[(dram2 >> 4) & 0x0F];     // i + 2;
                _dmaLineBuffer[n + 1] = LUT2[dram2 & 0x0F];        // i + 3;
                _dmaLineBuffer[n + 2] = LUT2[(dram1 >> 4) & 0x0F]; // i;
                _dmaLineBuffer[n + 3] = LUT2[dram1 & 0x0F];        // i + 1;
                DMemoryNewPtr -= 2;
            }
            // Send the data using I2S DMA driver.
            sendDataI2S(myI2S, _dmaI2SDesc);
            vscan_end();
        }
        delayMicroseconds(230);
    }

    for (int k = 0; k < 1; ++k)
    {
        vscan_start();
        for (int i = 0; i < E_INK_HEIGHT; ++i)
        {
            for (int n = 0; n < (E_INK_WIDTH / 4); n += 4)
            {
                _dmaLineBuffer[n] = 0;
                _dmaLineBuffer[n + 1] = 0;
                _dmaLineBuffer[n + 2] = 0;
                _dmaLineBuffer[n + 3] = 0;
            }
            // Send the data using I2S DMA driver.
            sendDataI2S(myI2S, _dmaI2SDesc);
            vscan_end();
        }
        delayMicroseconds(230);
    }

    vscan_start();
    if (!leaveOn)
        einkOff();

    _blockPartial = 0;
}

/**
 * @brief       partialUpdate function updates changed parts of the screen
 * without need to refresh whole display
 *
 * @param       bool _forced
 *              For advanced use with deep sleep. Can force partial update in
 * deep sleep
 *
 * @param       bool leaveOn
 *              if set to 1, it will disable turning supply for eink after
 *              display update in order to save some time needed for power supply
 *              to save some time at next display update or increase refreshing speed
 *
 * @note        Partial update only works in black and white mode
 *
 * @return      Number of pixels changed from black to white, leaving blur
 */
uint32_t EPDDriver::partialUpdate(bool _forced, bool leaveOn)
{
    if (getDisplayMode() == 1)
        return 0;

    if (_blockPartial == 1 && !_forced)
    {
        display1b(leaveOn);
        return 0;
    }

    if (_partialUpdateCounter >= _partialUpdateLimiter && _partialUpdateLimiter != 0)
    {
        // Force full update.
        display1b(leaveOn);

        // Reset the counter!
        _partialUpdateCounter = 0;

        // Go back!
        return 0;
    }

    uint16_t _pos = (E_INK_WIDTH * E_INK_HEIGHT / 8) - 1;
    uint32_t _send;
    uint8_t data = 0;
    uint8_t diffw, diffb;
    uint32_t n = (E_INK_WIDTH * E_INK_HEIGHT / 4) - 1;

    uint32_t changeCount = 0;

    _dmaI2SDesc->size = (E_INK_WIDTH / 4) + 16;
    _dmaI2SDesc->length = (E_INK_WIDTH / 4) + 16;
    _dmaI2SDesc->sosf = 1;
    _dmaI2SDesc->owner = 1;
    _dmaI2SDesc->qe.stqe_next = 0;
    _dmaI2SDesc->eof = 1;
    _dmaI2SDesc->buf = _dmaLineBuffer;
    _dmaI2SDesc->offset = 0;

    for (int i = 0; i < E_INK_HEIGHT; ++i)
    {
        for (int j = 0; j < E_INK_WIDTH / 8; ++j)
        {
            diffw = *(DMemoryNew + _pos) & ~*(_partial + _pos);
            diffb = ~*(DMemoryNew + _pos) & *(_partial + _pos);
            if (diffw) // count pixels turning from black to white as these are visible blur
            {
                for (int bv = 1; bv < 256; bv <<= 1)
                {
                    if (diffw & bv)
                        ++changeCount;
                }
            }
            _pos--;
            *(_pBuffer + n) = LUTW[diffw >> 4] & (LUTB[diffb >> 4]);
            n--;
            *(_pBuffer + n) = LUTW[diffw & 0x0F] & (LUTB[diffb & 0x0F]);
            n--;
        }
    }

    if (!einkOn())
        return 0;

// How many cycles / phases / frames to write to the panel.
#if defined(ARDUINO_ESP32_DEV)
    int rep = 5;
#elif defined(ARDUINO_INKPLATE6V2)
    int rep = 6;
#endif

    for (int k = 0; k < rep; ++k)
    {
        vscan_start();
        n = (E_INK_WIDTH * E_INK_HEIGHT / 4) - 1;
        for (int i = 0; i < E_INK_HEIGHT; ++i)
        {
            for (int j = 0; j < (E_INK_WIDTH / 4); j += 4)
            {
                _dmaLineBuffer[j + 2] = *(_pBuffer + n);
                _dmaLineBuffer[j + 3] = *(_pBuffer + n - 1);
                _dmaLineBuffer[j] = *(_pBuffer + n - 2);
                _dmaLineBuffer[j + 1] = *(_pBuffer + n - 3);
                n -= 4;
            }
            // Send the data using I2S DMA driver.
            sendDataI2S(myI2S, _dmaI2SDesc);
            vscan_end();
        }
        delayMicroseconds(230);
    }
    clean(2, 2);
    clean(3, 1);
    vscan_start();

    if (!leaveOn)
        einkOff();

    memcpy(DMemoryNew, _partial, E_INK_WIDTH * E_INK_HEIGHT / 8);

    if (_partialUpdateLimiter != 0)
        _partialUpdateCounter++;

    return changeCount;
}


/**
 * @brief   Set the number of partial updates afterwhich full screen update is performed.
 *
 * @param   uint16_t _numberOfPartialUpdates
 *          Number of allowed partial updates afterwhich full update is performed.
 *          0 = disabled, no automatic full update will be performed.
 *
 * @note    By default, this is disabled, but to keep best image quality perform a full update
 *          every 60-80 partial updates.
 */
void EPDDriver::setFullUpdateThreshold(uint16_t _numberOfPartialUpdates)
{
    // Copy the value into the local variable.
    _partialUpdateLimiter = _numberOfPartialUpdates;

    // If the limiter is enabled, force full update.
    if (_numberOfPartialUpdates != 0)
        _blockPartial = 1;
}

/**
 * @brief       einkOn turns on supply for epaper display (TPS65186) [+15 VDC,
 * -15VDC, +22VDC, -20VDC, +3.3VDC, VCOM]
 *
 * @note        its important to use this order when turning epaper on.
 *              using wrong order can irreparably damage epaper
 *
 * @return      1 if succesfully turned on, 0 if failed
 */
int EPDDriver::einkOn()
{
    if (getPanelState() == 1)
        return 1;
    WAKEUP_SET;
    delay(5);
    // Enable all rails
    Wire.beginTransmission(0x48);
    Wire.write(0x01);
    Wire.write(B00100000);
    Wire.endTransmission();


    // Modify power up sequence.
    Wire.beginTransmission(0x48);
    Wire.write(0x09);
    Wire.write(B11100100);
    Wire.endTransmission();

    // Modify power down sequence  (VEE and VNEG are swapped)
    Wire.beginTransmission(0x48);
    Wire.write(0x0b);
    Wire.write(B00011011);
    Wire.endTransmission();


    pinsAsOutputs();
    LE_CLEAR;
    SPH_SET;
    GMOD_SET;
    SPV_SET;
    CKV_CLEAR;
    OE_CLEAR;
    PWRUP_SET;
    setPanelState(1);

    unsigned long timer = millis();
    do
    {
        delay(1);
    } while ((readPowerGood() != PWR_GOOD_OK) && (millis() - timer) < 250);
    if ((millis() - timer) >= 250)
    {
        einkOff();
        return 0;
    }

    VCOM_SET;
    OE_SET;

    return 1;
}

/**
 * @brief       einkOff turns off epaper power supply and put all digital IO
 * pins in high Z state
 */
void EPDDriver::einkOff()
{
    if (getPanelState() == 0)
        return;
    VCOM_CLEAR;
    OE_CLEAR;
    GMOD_CLEAR;
    LE_CLEAR;
    CKV_CLEAR;
    SPH_CLEAR;
    SPV_CLEAR;
    PWRUP_CLEAR;

    unsigned long timer = millis();
    do
    {
        delay(1);
    } while ((readPowerGood() != 0) && (millis() - timer) < 250);

    WAKEUP_CLEAR; // Disable 3V3 Switch for ePaper.
    Wire.beginTransmission(0x48);
    Wire.write(0x01);
    Wire.write(B00000000);
    Wire.endTransmission();
    pinsZstate();
    setPanelState(0);
}

void EPDDriver::pmicBegin()
{
    WAKEUP_SET;
    delay(5);
    Wire.beginTransmission(0x48);
    Wire.write(0x09);
    Wire.write(0B00011011); // Power up seq.
    Wire.write(0B00000000); // Power up delay (3mS per rail)
    Wire.write(0B00011011); // Power down seq.
    Wire.write(0B00000000); // Power down delay (6mS per rail)
    Wire.endTransmission();
    delay(5);
    WAKEUP_CLEAR;
}


/**
 * @brief       pinsAsOutputs sets all tps pins as outputs
 */
void EPDDriver::pinsAsOutputs()
{
    pinMode(2, OUTPUT);
    pinMode(32, OUTPUT);
    pinMode(33, OUTPUT);
    internalIO.pinMode(OE, OUTPUT);
    internalIO.pinMode(GMOD, OUTPUT);
    internalIO.pinMode(SPV, OUTPUT);

    // Set up the EPD Data and CL pins for I2S.
    setI2S1pin(0, I2S1O_BCK_OUT_IDX, 0);
    setI2S1pin(4, I2S1O_DATA_OUT0_IDX, 0);
    setI2S1pin(5, I2S1O_DATA_OUT1_IDX, 0);
    setI2S1pin(18, I2S1O_DATA_OUT2_IDX, 0);
    setI2S1pin(19, I2S1O_DATA_OUT3_IDX, 0);
    setI2S1pin(23, I2S1O_DATA_OUT4_IDX, 0);
    setI2S1pin(25, I2S1O_DATA_OUT5_IDX, 0);
    setI2S1pin(26, I2S1O_DATA_OUT6_IDX, 0);
    setI2S1pin(27, I2S1O_DATA_OUT7_IDX, 0);

    // Start sending clock to the EPD.
    myI2S->conf1.tx_stop_en = 1;
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
 * @brief       readPowerGood reads ok status for each rail
 *
 * @return      power good status register
 */
uint8_t EPDDriver::readPowerGood()
{
    Wire.beginTransmission(0x48);
    Wire.write(0x0F);
    Wire.endTransmission();
    Wire.requestFrom(0x48, 1);
    return Wire.read();
}

/**
 * @brief       pinsZstate sets all tps pins at high z state
 *
 * @note        this is used only when turning off epaper
 */
void EPDDriver::pinsZstate()
{
    pinMode(2, INPUT);
    pinMode(32, INPUT);
    pinMode(33, INPUT);
    internalIO.pinMode(OE, INPUT);
    internalIO.pinMode(GMOD, INPUT);
    internalIO.pinMode(SPV, INPUT);

    // Set up the EPD Data and CL pins for I2S .
    pinMode(0, INPUT);
    pinMode(4, INPUT);
    pinMode(5, INPUT);
    pinMode(18, INPUT);
    pinMode(19, INPUT);
    pinMode(23, INPUT);
    pinMode(25, INPUT);
    pinMode(26, INPUT);
    pinMode(27, INPUT);

    myI2S->conf1.tx_stop_en = 0;
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
void EPDDriver::clean(uint8_t c, uint8_t rep)
{
    einkOn();
    uint8_t data = 0;
    if (c == 0)
        data = B10101010;
    else if (c == 1)
        data = B01010101;
    else if (c == 2)
        data = B00000000;
    else if (c == 3)
        data = B11111111;

    // Fill up the buffer with the data.
    for (int i = 0; i < (E_INK_WIDTH / 4); i++)
    {
        _dmaLineBuffer[i] = data;
    }

    _dmaI2SDesc->size = (E_INK_WIDTH / 4) + 16;
    _dmaI2SDesc->length = (E_INK_WIDTH / 4) + 16;
    _dmaI2SDesc->sosf = 1;
    _dmaI2SDesc->owner = 1;
    _dmaI2SDesc->qe.stqe_next = 0;
    _dmaI2SDesc->eof = 1;
    _dmaI2SDesc->buf = _dmaLineBuffer;
    _dmaI2SDesc->offset = 0;

    for (int k = 0; k < rep; ++k)
    {
        vscan_start();
        for (int i = 0; i < E_INK_HEIGHT; ++i)
        {
            // Send the data using I2S DMA driver.
            sendDataI2S(myI2S, _dmaI2SDesc);
            vscan_end();
        }
        delayMicroseconds(230);
    }
}

/**
 * @brief       hscan_start starts writing data into current row
 *
 * @param       uint32_t _d
 *              data to be written into current row
 */
void EPDDriver::hscan_start(uint32_t _d)
{
}

uint8_t EPDDriver::getDisplayMode()
{
    return _displayMode;
}

/**
 * @brief       Initializes the internal and external IO expanders,
 *              Configures all of the data and control pins for the
 *              EPD Driver.
 */
void EPDDriver::gpioInit()
{

    internalIO.begin(IO_INT_ADDR);
    internalIO.digitalWrite(9, LOW);
    externalIO.begin(IO_EXT_ADDR);

    for (uint32_t i = 0; i < 256; ++i)
        pinLUT[i] = ((i & B00000011) << 4) | (((i & B00001100) >> 2) << 18) | (((i & B00010000) >> 4) << 23) |
                    (((i & B11100000) >> 5) << 25);

    // Set all IO expander registers to 0
    memset(internalIO._ioExpanderRegs, 0, 22);
    memset(externalIO._ioExpanderRegs, 0, 22);

    internalIO.pinMode(VCOM, OUTPUT);
    internalIO.pinMode(PWRUP, OUTPUT);
    internalIO.pinMode(WAKEUP, OUTPUT);
    internalIO.pinMode(GPIO0_ENABLE, OUTPUT);
    internalIO.digitalWrite(GPIO0_ENABLE, 1);

    // Initialize I2C communication with the TPS chip
    pmicBegin();

    // Set all pins of seconds I/O expander to outputs, low.
    // For some reason, it draw more current in deep sleep when pins are set as
    // inputs...
    for (int i = 0; i < 15; i++)
    {
        externalIO.pinMode(i, OUTPUT);
        externalIO.digitalWrite(i, LOW);
    }

    // For same reason, unused pins of first I/O expander have to be also set as
    // outputs, low.
    internalIO.pinMode(14, OUTPUT);
    internalIO.pinMode(15, OUTPUT);
    internalIO.digitalWrite(14, LOW);
    internalIO.digitalWrite(15, LOW);

    // Set SPI pins to input to reduce power consumption in deep sleep
    pinMode(12, INPUT);
    pinMode(13, INPUT);
    pinMode(14, INPUT);
    pinMode(15, INPUT);

    // And also disable uSD card supply
    internalIO.pinMode(SD_PMOS_PIN, INPUT);
}

/**
 * @brief       initializeFramebuffers allocates memory to be used
 *              by specific display modes of the display
 *
 * @return      returns 0 if allocation failed, 1 if it succeeded
 */
uint8_t EPDDriver::initializeFramebuffers()
{
    // Initialize all the framebuffers
    DMemoryNew = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 8);
    _partial = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 8);
    _pBuffer = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 4);
    DMemory4Bit = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 2);
    GLUT = (uint32_t *)malloc(256 * 9 * sizeof(uint32_t));
    GLUT2 = (uint32_t *)malloc(256 * 9 * sizeof(uint32_t));
    if (DMemoryNew == NULL || _partial == NULL || _pBuffer == NULL || DMemory4Bit == NULL || GLUT == NULL ||
        GLUT2 == NULL)
    {
        return 0;
    }
    memset(DMemoryNew, 0, E_INK_WIDTH * E_INK_HEIGHT / 8);
    memset(_partial, 0, E_INK_WIDTH * E_INK_HEIGHT / 8);
    memset(_pBuffer, 0, E_INK_WIDTH * E_INK_HEIGHT / 4);
    memset(DMemory4Bit, 255, E_INK_WIDTH * E_INK_HEIGHT / 2);

    return 1;
}

/**
 * @brief       sdCardInit initializes sd card trough SPI
 *
 * @return      0 if failed to initialise, 1 if successful
 */
int16_t EPDDriver::sdCardInit()
{
    internalIO.pinMode(SD_PMOS_PIN, OUTPUT);
    internalIO.digitalWrite(SD_PMOS_PIN, LOW);
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
    internalIO.pinMode(SD_PMOS_PIN, INPUT);
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
    internalIO.pinMode(9, INPUT);
    int state = internalIO.digitalRead(9);
    internalIO.pinMode(9, OUTPUT);

    // If the input is pulled high, it's PMOS only.
    // If it's pulled low, it's PMOS and NMOS.
    if (state)
    {
        internalIO.digitalWrite(9, LOW);
    }
    else
    {
        internalIO.digitalWrite(9, HIGH);
    }

    // Wait a little bit after a MOSFET enable.
    delay(5);

    // Set to the highest resolution and read the voltage.
    analogReadResolution(12);
    int adc = analogReadMilliVolts(35);

    // Turn off the MOSFET (and voltage divider).
    if (state)
    {
        internalIO.digitalWrite(9, HIGH);
    }
    else
    {
        internalIO.digitalWrite(9, LOW);
    }

    // Calculate the voltage at the battery terminal (voltage is divided in half by voltage divider).
    return (double(adc) * 2.0 / 1000);
}

/**
 * @brief       readTemperature reads panel temperature
 *
 * @return      returns  temperature in range from -10 to 85 degree C with
 * accuracy of +-1 in range from 0 to 50
 */
int8_t EPDDriver::readTemperature()
{
    int8_t temp;
    if (getPanelState() == 0)
    {
        WAKEUP_SET;
        PWRUP_SET;
        delay(5);
    }
    Wire.beginTransmission(0x48);
    Wire.write(0x0D);
    Wire.write(B10000000);
    Wire.endTransmission();
    delay(5);

    Wire.beginTransmission(0x48);
    Wire.write(0x00);
    Wire.endTransmission();

    Wire.requestFrom(0x48, 1);
    temp = Wire.read();
    if (getPanelState() == 0)
    {
        PWRUP_CLEAR;
        WAKEUP_CLEAR;
        delay(5);
    }
    return temp;
}

#endif