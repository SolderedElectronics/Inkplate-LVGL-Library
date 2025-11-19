/**
 **************************************************
 *
 * @file        EEPROMUsage.ino
 * @brief       Example showing how to clear, write to and read from the onboard EEPROM
 *              and display the result using LVGL on Inkplate 10.
 *
 *              EEPROM is a non-volatile memory that keeps its content even when
 *              power is removed. This example:
 *                1) Clears 128 user bytes in EEPROM (starting at address 76)
 *                2) Writes incremental values (0..127)
 *                3) Reads them back and shows them on the e-paper display
 *
 * For info on how to quickly get started with Inkplate 10 visit:
 * https://soldered.com/documentation/inkplate/10/overview/
 *
 * CAUTION! Changing EEPROM size can wipe waveform data.
 * CAUTION! EEPROM addresses from 0 to 75 are used for waveform. DO NOT WRITE OR MODIFY DATA ON THESE ADDRESSES!
 *
 * @authors     Soldered
 * @date        November 2025
 **************************************************
 */

#if !defined(ARDUINO_INKPLATE10) && !defined(ARDUINO_INKPLATE10V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate10 or Soldered Inkplate10 in the boards menu."
#endif

#include <EEPROM.h>        // ESP32 EEPROM library
#include <Inkplate-LVGL.h> // Inkplate + LVGL

// Start EEPROM address for user data. Addresses below 76 are waveform data!
#define EEPROM_START_ADDR 76
#define DATA_SIZE         128
#define EEPROM_SIZE       (EEPROM_START_ADDR + DATA_SIZE)

// Inkplate in 1-bit (black & white) mode
Inkplate inkplate(INKPLATE_1BIT);

// LVGL label used to show EEPROM status/content
static lv_obj_t *lbl_text = nullptr;

// Forward declarations
void clearEEPROM();
void writeEEPROM();
void printEEPROM();

// Small helper to show text on the display via LVGL
static void showText(const char *text)
{
    lv_label_set_text(lbl_text, text);
    lv_obj_align(lbl_text, LV_ALIGN_TOP_LEFT, 8, 8);

    lv_tick_inc(20);
    lv_timer_handler();
    inkplate.display();   // FULL refresh (done only a few times)
}

void setup()
{
    Serial.begin(115200);
    delay(200);
    Serial.println("Starting EEPROMUsage LVGL example on Inkplate 10...");

    // Initialize Inkplate + LVGL in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // Init EEPROM with full size (reserved + user)
    if (!EEPROM.begin(EEPROM_SIZE))
    {
        Serial.println("EEPROM init failed!");
        // Still try to show something on display
    }

    // White background
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);

    // Create a label for status + data
    lbl_text = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(lbl_text, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(lbl_text, &lv_font_montserrat_20, 0);
    lv_label_set_long_mode(lbl_text, LV_LABEL_LONG_WRAP);
    // Inkplate 10 width is 1200 px; leave small margins
    lv_obj_set_width(lbl_text, 1200 - 16);

    // 1) Clear user region
    Serial.println("Clearing EEPROM user region...");
    showText("Clearing EEPROM user region...");
    clearEEPROM();
    delay(500);

    // 2) Write data
    Serial.println("Writing data to EEPROM user region...");
    showText("Writing data to EEPROM user region...");
    writeEEPROM();
    delay(500);

    // 3) Read back and display
    Serial.println("Reading data from EEPROM user region...");
    printEEPROM();  // This will also update the LVGL label
    delay(500);
}

void loop()
{
    // Nothing to do here, example runs once in setup()
}

// Function for clearing user EEPROM data (does NOT touch waveform region 0–75)
void clearEEPROM()
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        // Start writing from EEPROM_START_ADDR, keep waveform data intact
        EEPROM.write(EEPROM_START_ADDR + i, 0);
    }
    EEPROM.commit();
}

// Function writes data to EEPROM user region
void writeEEPROM()
{
    for (int i = 0; i < DATA_SIZE; i++)
    {
        EEPROM.write(EEPROM_START_ADDR + i, i); 
    }
    EEPROM.commit();
}

// Function reads back previously written data and displays it on serial monitor and screen.
void printEEPROM()
{
    // Build a single text buffer for the display
    String txt = "EEPROM user contents (0..127)\n"
                 "stored at addresses 76..203:\n";

    for (int i = 0; i < DATA_SIZE; i++)
    {
        uint8_t val = EEPROM.read(EEPROM_START_ADDR + i);

        // Print to Serial
        Serial.print(val, DEC);
        if (i != DATA_SIZE - 1)
        {
            Serial.print(", ");
        }

        // Append to display string
        txt += String(val);
        if (i != DATA_SIZE - 1)
            txt += ", ";
    }
    Serial.println();

    // Show on LVGL label
    showText(txt.c_str());
}
