/**
 **************************************************
 *
 * @file        Inkplate13Driver.cpp
 * @brief       Low-level EPD driver implementation for Inkplate 13SPECTRA
 *
 *
 * @copyright   GNU General Public License v3.0
 * @authors     Soldered
 ***************************************************/

// Header guard for the Arduino include
#ifdef ARDUINO_INKPLATE13SPECTRA
#include "Inkplate13SPECTRADriver.h"
#include "Inkplate-LVGL.h"
#include "../../system/inkplateSemaphore.h"


// SPI used for the MicroSd card
SPIClass spi1(1);

// Instance of the SdFat object
SdFat sd;

// SPI settings for communicating with the display, 10MHz
SPISettings epdSpiSettings(10000000, MSBFIRST, SPI_MODE0);

/**
 *
 * @brief       writePixelInternal funtion sets pixel data for (x, y) pixel position
 *
 * @param       int16_t x0
 *              default position for x, will be changed depending on rotation
 * @param       int16_t y0
 *              default position for y, will be changed depending on rotation
 * @param       uint16_t color
 *              pixel color
 *
 * @note        If x0 or y0 are out of inkplate screen borders, function will
 * exit.
 */
void EPDDriver::writePixelInternal(int16_t x, int16_t y, uint16_t color)
{
    int16_t x0 = x;
    int16_t y0 = y;
    if (color > 5)
        return;
    color = colorPalette[color];

    _swap_int16_t(x0, y0);
    y0 = E_INK_HEIGHT - y0 - 1;
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

    if (self->ditherEnabled && self->_renderMode == LV_DISP_RENDER_MODE_FULL)
    {
        self->dither.ditherFramebuffer(px_map, E_INK_HEIGHT, E_INK_WIDTH);
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

                // Classification to Inkplate 6-color palette: nearest palette entry by
                // luminance-weighted RGB distance. A hue/saturation classifier was used
                // here previously, but saturation (delta/maxc) blows up on noise for
                // near-black pixels (maxc close to zero), misrouting them into the hue
                // cascade's RED fallback instead of BLACK. Plain RGB distance has no such
                // instability and needs no float trig.
                static const struct
                {
                    uint8_t r, g, b, code;
                } paletteRGB[6] = {
                    {0, 0, 0, INKPLATE_BLACK}, {255, 255, 255, INKPLATE_WHITE}, {255, 255, 0, INKPLATE_YELLOW},
                    {255, 0, 0, INKPLATE_RED}, {0, 0, 255, INKPLATE_BLUE},      {0, 255, 0, INKPLATE_GREEN},
                };

                uint8_t color = INKPLATE_BLACK;
                uint32_t bestDist = UINT32_MAX;
                for (uint8_t j = 0; j < 6; j++)
                {
                    int32_t dr = (int32_t)R - paletteRGB[j].r;
                    int32_t dg = (int32_t)G - paletteRGB[j].g;
                    int32_t db = (int32_t)B - paletteRGB[j].b;
                    uint32_t dist = (uint32_t)(dr * dr) * 30 + (uint32_t)(dg * dg) * 59 + (uint32_t)(db * db) * 11;
                    if (dist < bestDist)
                    {
                        bestDist = dist;
                        color = paletteRGB[j].code;
                    }
                }

                // Apply Inkplate 13 framebuffer orientation
                int32_t sx = area->x1 + x;
                int32_t sy = area->y1 + y;
                int32_t fx = sy;
                int32_t fy = E_INK_HEIGHT - sx - 1;

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

    // Check if begin is called already. If it is, that means that panel needs to
    // be reseted in order to be woken up from deep sleep. If is this first time
    // begin has been called, init SPI, init pins, allocate memory for frame
    // buffer and clear frame buffer
    if (!_beginDone)
    {
        setPanelPinsToLow();


        Wire.begin();

        // Save the given inkplate pointer for internal use
        _inkplate = _inkplatePtr;


        expander1.begin(IO_INT_ADDR);

        _inkplate->setRotation(1);

        // Allocate memory for internal frame buffer
        DMemory4Bit = (uint8_t *)ps_malloc(E_INK_WIDTH * E_INK_HEIGHT / 2);
        if (DMemory4Bit == NULL)
        {
            return false;
        }

        dither.begin(_palette, _paletteIndex, paletteSize, _inkplatePtr);

        // Color whole frame buffer in white color
        memset(DMemory4Bit, INKPLATE_WHITE | (INKPLATE_WHITE << 4), E_INK_WIDTH * E_INK_HEIGHT / 2);


        _beginDone = true;
    }

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
    displayStart();
    spiStart();
    // Power up the screen (if is not already powered on).
    setPanelState(true);

    // Framebuffer must be send to two seperate display driver.
    // The screen is splitted in half - left and right side.
    // Send data in rows - first on master (left side of the screen).
    digitalWrite(SPECTRA133_CS_M_PIN, LOW);
    digitalWrite(SPECTRA133_CS_S_PIN, HIGH);

    // Start SPI transaction and send the command to fill the EPD framebuffer with data.
    SPI.beginTransaction(epdSpiSettings);
    SPI.write(SPECTRA133_REGISTER_DTM);

    // Send the data to the first driver (left side)
    for (int i = 0; i < E_INK_HEIGHT; i++)
    {
        SPI.writeBytes(DMemory4Bit + (i * E_INK_WIDTH / 2), (E_INK_WIDTH / 4));
    }
    SPI.endTransaction();

    // Send data in rows - now on slave (right side of the screen).
    digitalWrite(SPECTRA133_CS_M_PIN, HIGH);
    digitalWrite(SPECTRA133_CS_S_PIN, LOW);
    waitForBusy();

    // Start SPI transaction and send the command to fill the EPD framebuffer with data.
    SPI.beginTransaction(epdSpiSettings);
    SPI.write(SPECTRA133_REGISTER_DTM);

    // Send the data to the second driver (right side).
    for (int i = 0; i < E_INK_HEIGHT; i++)
    {
        SPI.writeBytes(DMemory4Bit + (i * E_INK_WIDTH / 2) + (E_INK_WIDTH / 4), (E_INK_WIDTH / 4));
    }
    SPI.endTransaction();

    // Disable both drivers.
    digitalWrite(SPECTRA133_CS_S_PIN, HIGH);
    digitalWrite(SPECTRA133_CS_M_PIN, HIGH);
    waitForBusy();

    // Force display command.
    sendCommand(SPECTRA133_REGISTER_DRF, SPECTRA133_REGISTER_DRF_V, sizeof(SPECTRA133_REGISTER_DRF_V), eChipIdBoth);
    waitForBusy();

    // Disable power to the display (if needed).
    if (!_leaveOn)
        setPanelState(false);
    spiEnd();
    displayEnd();
}

/**
 * @brief       returns the current panel state, 0 for off, 1 for on
 */
uint8_t EPDDriver::getPanelState()
{
    return _panelState;
}

/**
 * @brief       sets the current panel state
 *
 * @param       uint8_t state
 *              if set to 1, the panel will be turned on and initialized.
 *              is et to 0, the panel will be turned off.
 */
void EPDDriver::setPanelState(uint8_t state)
{
    // Check if the current display power state is different than the provided.
    // If there is a difference - update it/change it.
    if (state != _panelState)
    {
        // Check if the screen must be powered down or powered up.
        if (state)
        {

            setPanelPinsToLow();

            delay(50);

            // Configure GPIOs.
            setIO();

            // Enable power to the screen.
            digitalWrite(SPECTRA133_PWR_EN, HIGH);

            // Wait a little bit.
            delay(100ULL);

            // First, do a hardware reset!
            resetPanel();

            // Wait for the reset to finish.
            delay(100ULL);

            // Initialze the screen by sending the magic values to the registers provided by the manufacturer.
            screenInit();

            // Power up a screen.
            sendCommand(SPECTRA133_REGISTER_PON, nullptr, 0, eChipIdBoth);
            waitForBusy();
        }
        else
        {
            // Power off the screen.
            sendCommand(SPECTRA133_REGISTER_POF, SPECTRA133_REGISTER_POF_V, sizeof(SPECTRA133_REGISTER_POF_V),
                        eChipIdBoth);
            waitForBusy();

            // Disable GPIOs.
            pinMode(SPECTRA133_DC_PIN, INPUT);
            pinMode(SPECTRA133_CS_M_PIN, INPUT);
            pinMode(SPECTRA133_CS_S_PIN, INPUT);
            pinMode(SPECTRA133_RST_PIN, INPUT);
            pinMode(SPECTRA133_BUSYN_PIN, INPUT);
            pinMode(SPECTRA133_PWR_EN, INPUT);

            // Disable power to the screen.
            digitalWrite(SPECTRA133_PWR_EN, LOW);
        }

        // Update the status variable.
        _panelState = state;
    }
}

/**
 * @brief       setIO initializes the communication pins as well as SPI communication with the Inkplate 13 panel
 */
void EPDDriver::setIO()
{
    // Config the GPIOs.
    pinMode(SPECTRA133_DC_PIN, OUTPUT);
    pinMode(SPECTRA133_CS_M_PIN, OUTPUT);
    pinMode(SPECTRA133_CS_S_PIN, OUTPUT);
    pinMode(SPECTRA133_RST_PIN, OUTPUT);
    pinMode(SPECTRA133_BUSYN_PIN, INPUT_PULLUP);
    pinMode(SPECTRA133_PWR_EN, OUTPUT);
    pinMode(SPECTRA133_BS0, OUTPUT);
    pinMode(SPECTRA133_BS1, OUTPUT);

    // Set their default states.
    digitalWrite(SPECTRA133_DC_PIN, HIGH);
    digitalWrite(SPECTRA133_CS_M_PIN, HIGH);
    digitalWrite(SPECTRA133_CS_S_PIN, HIGH);
    digitalWrite(SPECTRA133_RST_PIN, LOW);
    digitalWrite(SPECTRA133_PWR_EN, LOW);
    digitalWrite(SPECTRA133_BS0, LOW);
    digitalWrite(SPECTRA133_BS1, HIGH);

    // Config SPI.
    if (!SPI.begin(SPECTRA133_SPI_SCK, SPECTRA133_SPI_MISO, SPECTRA133_SPI_MOSI))
    {
        Serial.println("Failed to init SPI");
    }
}

/**
 * @brief       resetPanel resets Inkplate 13 screen
 */
void EPDDriver::resetPanel()
{
    // Toggle the reset pin to initialte HW reset.
    digitalWrite(SPECTRA133_RST_PIN, LOW);
    delay(100ULL);
    digitalWrite(SPECTRA133_RST_PIN, HIGH);
    delay(100ULL);
}

/**
 * @brief       sendCommand sends SPI command to Inkplate 13
 *
 * @param       uint8_t _command
 *              predefined command for epaper control
 */
void EPDDriver::sendCommand(uint8_t _cmd, const uint8_t *_parameters, uint32_t _n, enum eSpectraChipID _chipId)
{
    // Config the SPI.
    SPI.beginTransaction(epdSpiSettings);

    // Set the chip select pin to low as well.
    if (_chipId & eChipIdSlave)
        digitalWrite(SPECTRA133_CS_S_PIN, LOW);
    if (_chipId & eChipIdMaster)
        digitalWrite(SPECTRA133_CS_M_PIN, LOW);

    // Send the command.
    SPI.write(_cmd);

    if (_n != 0)
    {
        SPI.writeBytes(_parameters, _n);
    }

    SPI.endTransaction();

    // Release the chip select.
    if (_chipId & eChipIdSlave)
        digitalWrite(SPECTRA133_CS_S_PIN, HIGH);
    if (_chipId & eChipIdMaster)
        digitalWrite(SPECTRA133_CS_M_PIN, HIGH);
}

/**
 * @brief       screenInit powers up and initializes the E ink panel
 */
void EPDDriver::screenInit()
{
    // Send magic values to the registers. These values are provided from the manufacturer.
    sendCommand(SPECTRA133_REGISTER_AN_TM, SPECTRA133_REGISTER_AN_TM_V, sizeof(SPECTRA133_REGISTER_AN_TM_V),
                eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_CMD66, SPECTRA133_REGISTER_CMD66_V, sizeof(SPECTRA133_REGISTER_CMD66_V),
                eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_PSR, SPECTRA133_REGISTER_PSR_V, sizeof(SPECTRA133_REGISTER_PSR_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_PLL, SPECTRA133_REGISTER_PLL_V, sizeof(SPECTRA133_REGISTER_PLL_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_CDI, SPECTRA133_REGISTER_CDI_V, sizeof(SPECTRA133_REGISTER_CDI_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_TCON, SPECTRA133_REGISTER_TCON_V, sizeof(SPECTRA133_REGISTER_TCON_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_AGID, SPECTRA133_REGISTER_AGID_V, sizeof(SPECTRA133_REGISTER_AGID_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_PWS, SPECTRA133_REGISTER_PWS_V, sizeof(SPECTRA133_REGISTER_PWS_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_CCSET, SPECTRA133_REGISTER_CCSET_V, sizeof(SPECTRA133_REGISTER_CCSET_V),
                eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_TRES, SPECTRA133_REGISTER_TRES_V, sizeof(SPECTRA133_REGISTER_TRES_V), eChipIdBoth);
    sendCommand(SPECTRA133_REGISTER_PWR, SPECTRA133_REGISTER_PWR_V, sizeof(SPECTRA133_REGISTER_PWR_V), eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_EN_BUF, SPECTRA133_REGISTER_EN_BUF_V, sizeof(SPECTRA133_REGISTER_EN_BUF_V),
                eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_BTST_P, SPECTRA133_REGISTER_BTST_P_V, sizeof(SPECTRA133_REGISTER_BTST_P_V),
                eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_BOOST_VDDP_EN, SPECTRA133_REGISTER_BOOST_VDDP_EN_V,
                sizeof(SPECTRA133_REGISTER_BOOST_VDDP_EN_V), eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_BTST_N, SPECTRA133_REGISTER_BTST_N_V, sizeof(SPECTRA133_REGISTER_BTST_N_V),
                eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_BUCK_BOOST_VDDN, SPECTRA133_REGISTER_BUCK_BOOST_VDDN_V,
                sizeof(SPECTRA133_REGISTER_BUCK_BOOST_VDDN_V), eChipIdMaster);
    sendCommand(SPECTRA133_REGISTER_TFT_VCOM_POWER, SPECTRA133_REGISTER_TFT_VCOM_POWER_V,
                sizeof(SPECTRA133_REGISTER_TFT_VCOM_POWER_V), eChipIdMaster);
}


/**
 * @brief       sdCardInit initializes sd card trough SPI
 *
 * @return      0 if failed to initialise, 1 if successful
 */
int16_t EPDDriver::sdCardInit()
{
    expander1.pinMode(SD_PMOS_PIN, OUTPUT);
    expander1.digitalWrite(SD_PMOS_PIN, LOW);
    delay(200);
    spiStart();
    spi1.begin(12, 13, 11, 10);
    setSdCardOk(sd.begin(SdSpiConfig(10, SHARED_SPI, SD_SCK_MHZ(25), &spi1)));
    spiEnd();
    // Small delay to init the SD card
    delay(100);
    return getSdCardOk();
}

/**
 * @brief       sdCardSleep turns off the P-MOS which powers the sd card to save energy in deep sleep
 */
void EPDDriver::sdCardSleep()
{
    spiStart();
    spiEnd();
    // Set SPI pins to input to reduce power consumption in deep sleep
    pinMode(10, INPUT);
    pinMode(11, INPUT);
    pinMode(12, INPUT);
    pinMode(13, INPUT);

    // And also disable uSD card supply
    expander1.pinMode(SD_PMOS_PIN, INPUT);
}

/**
 * @brief       getSdFat gets sd card object
 *
 * @return      sd card class object
 */
SdFat &EPDDriver::getSdFat()
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
    return &spi1;
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
    expander1.pinMode(9, INPUT);
    int state = expander1.digitalRead(9);
    expander1.pinMode(9, OUTPUT);

    // If the input is pulled high, it's PMOS only.
    // If it's pulled low, it's PMOS and NMOS.
    if (state)
    {
        expander1.digitalWrite(9, LOW);
    }
    else
    {
        expander1.digitalWrite(9, HIGH);
    }

    // Wait a little bit after a MOSFET enable.
    delay(5);

    // Set to the highest resolution and read the voltage.
    analogReadResolution(12);
    int adc = analogReadMilliVolts(1);

    // Turn off the MOSFET (and voltage divider).
    if (state)
    {
        expander1.digitalWrite(9, HIGH);
    }
    else
    {
        expander1.digitalWrite(9, LOW);
    }

    // Calculate the voltage at the battery terminal (voltage is divided in half by voltage divider).
    return (double(adc) * 2.0 / 1000);
}

/**
 * @brief       waitForBusy  waits until the screen is ready to accept new commands.
 */
void EPDDriver::waitForBusy()
{
    // Wait until the screen is ready to accept new commands.
    // This will be indicated by pulling the BUSYN pin to high.
    // A 30-second timeout prevents an infinite hang if the panel does not respond.
    while (!digitalRead(SPECTRA133_BUSYN_PIN))
    {
        // Let the RTOS breathe.
        delay(1);
    }
}

// Function helps empty capacitors, without this sometimes the panel refuses to refresh...
/**
 * @brief       setPanelPinsToLow helps empty capacitors, without this sometimes the panel refuses to refresh...
 */
void EPDDriver::setPanelPinsToLow()
{
    pinMode(SPECTRA133_DC_PIN, OUTPUT);
    pinMode(SPECTRA133_CS_M_PIN, OUTPUT);
    pinMode(SPECTRA133_RST_PIN, OUTPUT);
    pinMode(SPECTRA133_BUSYN_PIN, OUTPUT);
    pinMode(SPECTRA133_CS_S_PIN, OUTPUT);
    pinMode(SPECTRA133_PWR_EN, OUTPUT);
    pinMode(SPECTRA133_BS0, OUTPUT);
    pinMode(SPECTRA133_BS1, OUTPUT);

    digitalWrite(SPECTRA133_DC_PIN, LOW);
    digitalWrite(SPECTRA133_CS_M_PIN, LOW);
    digitalWrite(SPECTRA133_RST_PIN, LOW);
    digitalWrite(SPECTRA133_BUSYN_PIN, LOW);
    digitalWrite(SPECTRA133_CS_S_PIN, LOW);
    digitalWrite(SPECTRA133_PWR_EN, LOW);
    digitalWrite(SPECTRA133_BS0, LOW);
    digitalWrite(SPECTRA133_BS1, LOW);
}


#endif