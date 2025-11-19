/**
 **************************************************
 *
 * @file        EEPROMUsage.ino
 * @brief       Example showing how to clear, write to and read from the onboard EEPROM
 *              and display the result using LVGL on Inkplate 5V2.
 *
 *              EEPROM is a non-volatile memory that keeps its content even when
 *              power is removed. This example:
 *                1) Clears the first 128 bytes of EEPROM
 *                2) Writes incremental values (0..127)
 *                3) Reads them back and shows them on the e-paper display
 *
 * For setup instructions and more information about Inkplate 5V2 visit:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 **************************************************
 */

#include <EEPROM.h>        // ESP32 EEPROM library
#include <Inkplate-LVGL.h> // Inkplate + LVGL

#define EEPROM_SIZE 128    // How many bytes we touch in this example

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
    Serial.println("Starting EEPROMUsage LVGL example...");

    // Initialize Inkplate + LVGL in FULL render mode
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    // Initialize EEPROM with 128 bytes
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
    lv_obj_set_width(lbl_text, 800 - 16);

    // 1) Clear
    Serial.println("Clearing EEPROM...");
    showText("Clearing EEPROM...");
    clearEEPROM();
    delay(500);

    // 2) Write
    Serial.println("Writing data to EEPROM...");
    showText("Writing data to EEPROM...");
    writeEEPROM();
    delay(500);

    // 3) Read back and display
    Serial.println("Reading data from EEPROM...");
    printEEPROM();  // This will also update the LVGL label
    delay(500);
}

void loop()
{
    // Nothing to do here, example runs once in setup()
}

// Function for clearing EEPROM data
void clearEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, 0);
    }
    EEPROM.commit();
}

// Function writes data to EEPROM
void writeEEPROM()
{
    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        EEPROM.write(i, i); 
    }
    EEPROM.commit();
}

// Function reads back previously written data and displays it on serial monitor and screen.
void printEEPROM()
{
    // Build a single text buffer for the display
    String txt = "EEPROM contents (0..127):\n";

    for (int i = 0; i < EEPROM_SIZE; i++)
    {
        uint8_t val = EEPROM.read(i);

        // Print to Serial
        Serial.print(val, DEC);
        if (i != EEPROM_SIZE - 1)
        {
            Serial.print(", ");
        }

        // Append to display string
        txt += String(val);
        if (i != EEPROM_SIZE - 1)
            txt += ", ";
    }
    Serial.println();

    // Show on LVGL label
    showText(txt.c_str());
}
