/**
 **************************************************
 *
 * @file        SetVCOM_Inkplate10.ino
 * @brief       WARNING! - VCOM voltage is written in EEPROM, which means it can be
 *              set a limited number of times, so don't run this sketch repeatedly!
 *              VCOM should be set once and then left as is.
 *
 *              This example:
 *                - Reads the current VCOM value from the panel
 *                - Shows it on screen with a grayscale test pattern (LVGL)
 *                - Lets you type a new VCOM value over Serial ([-5.00, 0.00] V)
 *                - Programs it into the panel EEPROM (and stores it in ESP32 EEPROM too)
 *
 * For info on how to quickly get started with Inkplate 10 visit:
 * https://soldered.com/documentation/inkplate/10/overview/
 *
 * WARNING: Each VCOM write wears panel EEPROM. Use sparingly.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#if !defined(ARDUINO_INKPLATE10) && !defined(ARDUINO_INKPLATE10V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate10 or Soldered Inkplate10 in the boards menu."
#endif

#include <EEPROM.h>
#include <Inkplate-LVGL.h>
#include <Wire.h>

// Inkplate 10 in 3-bit grayscale mode
Inkplate inkplate(INKPLATE_3BIT);

double currentVCOM;          // Stores the current VCOM value read from panel
const int EEPROMAddress = 0; // Keep address 0 for correct VCOM storage in ESP32 EEPROM
double vcomVoltage;

// Forward declarations
double readPanelVCOM();
double getVCOMFromSerial(double *_vcom);
uint8_t writeVCOMToEEPROM(double v);
static inline int display_width();
static inline int display_height();
void displayTestImage();
void writeReg(uint8_t reg, float data);
uint8_t readReg(uint8_t reg);

void setup()
{
    Serial.begin(115200);     // Start serial at 115200 baud
    EEPROM.begin(512);        // Initialize ESP32 EEPROM
    Wire.begin();             // Initialize I2C bus
    inkplate.begin();         // Initialize Inkplate + LVGL backend

    Serial.println("The optimal VCOM Voltage for your Inkplate's panel can sometimes");
    Serial.println("be found written on the flat cable connector.");
    Serial.println("Write VCOM voltage from epaper panel.");
    Serial.println("Don't forget negative (-) sign!");
    Serial.println("Use dot as the decimal point.");
    Serial.println("For example: -1.23");
    Serial.println();

    // First screen: current VCOM + grayscale bars
    displayTestImage();
}

void loop()
{
    if (Serial.available())
    {
        // Read VCOM from Serial until it's in range [-5.0, 0.0]
        do
        {
            getVCOMFromSerial(&vcomVoltage);
            Serial.println(vcomVoltage, 2);

            if (vcomVoltage < -5.0 || vcomVoltage > 0.0)
            {
                Serial.println("VCOM out of range! [-5.0, 0.0]");
            }
        } while (vcomVoltage < -5.0 || vcomVoltage > 0.0);

        // Program the panel EEPROM
        // Internal IO pin 6 is used as the TPS65186 VCOM programming status pin
        inkplate.internalIO.pinMode(6, INPUT_PULLUP);

        if (writeVCOMToEEPROM(vcomVoltage))
        {
            EEPROM.put(EEPROMAddress, vcomVoltage);
            EEPROM.commit();
        }

        // Clear LVGL screen and redraw test image with updated VCOM value
        lv_obj_clean(lv_scr_act());
        displayTestImage();
    }
}

// ----- VCOM helpers -----

double readPanelVCOM()
{
    delay(10); // Give TPS65186 time so registers respond
    uint8_t vcomL = readReg(0x03);         // Low 8 bits from register 0x03
    uint8_t vcomH = readReg(0x04) & 0x01;  // Mask bit 0 (MSB of raw)
    delay(10);
    int raw = (vcomH << 8) | vcomL;        // 0 .. 511
    return -(raw / 100.0);                 // VCOM in volts (negative)
}

double getVCOMFromSerial(double *_vcom)
{
    double vcom = 0;
    char buff[32];
    unsigned long start;

    // Wait for first char
    while (!Serial.available())
        ;

    start = millis();
    int idx = 0;

    // Collect characters for up to 500 ms idle gap
    while ((millis() - start) < 500 && idx < (int)sizeof(buff) - 1)
    {
        if (Serial.available())
        {
            char c = Serial.read();
            buff[idx++] = c;
            start = millis();
        }
    }

    buff[idx] = '\0';
    sscanf(buff, "%lf", &vcom);
    *_vcom = vcom;
    return vcom;
}

uint8_t writeVCOMToEEPROM(double v)
{
    // Build a 9-bit raw value (0 - 511)
    int raw = int(abs(v) * 100) & 0x1FF;
    uint8_t lsb = raw & 0xFF;
    uint8_t msb = (raw >> 8) & 0x01;

    // NOTE: With Inkplate-LVGL, power control is handled internally.
    // We assume the TPS65186 is powered from inkplate.begin().

    writeReg(0x03, lsb);
    uint8_t r4 = readReg(0x04) & ~0x01;
    writeReg(0x04, r4 | msb);
    // Set bit 6 to trigger EEPROM programming
    writeReg(0x04, (r4 | msb) | (1 << 6));

    // Wait for VCOM programming to finish
    while (inkplate.internalIO.digitalRead(6))
    {
        delay(1);
    }

    // Clear interrupt flag and clean registers
    readReg(0x07);  // clear interrupt flag
    writeReg(0x03, 0);
    writeReg(0x04, 0);

    // We skip explicit einkOff/einkOn here because those are private
    // in the LVGL Inkplate10 driver and the driver takes care of power.

    // Verify written value
    uint8_t vL = readReg(0x03);
    uint8_t vH = readReg(0x04) & 0x01;
    int check = (vH << 8) | vL;

    if (check != raw)
    {
        Serial.printf("Verification failed: got %d, want %d\n", check, raw);
        return 0;
    }

    Serial.println("VCOM EEPROM PROGRAMMING OK");
    return 1;
}

// ----- TPS65186 I2C access -----

void writeReg(uint8_t reg, float data)
{
    Wire.beginTransmission(0x48);
    Wire.write(reg);
    Wire.write((uint8_t)data);
    Wire.endTransmission();
}

uint8_t readReg(uint8_t reg)
{
    Wire.beginTransmission(0x48);
    Wire.write(reg);
    Wire.endTransmission(false);
    Wire.requestFrom((uint8_t)0x48, (uint8_t)1);
    return Wire.read();
}

// ----- LVGL helpers -----

static inline int display_width()
{
    lv_display_t *disp = lv_disp_get_default();
    return lv_display_get_horizontal_resolution(disp);
}

static inline int display_height()
{
    lv_display_t *disp = lv_disp_get_default();
    return lv_display_get_vertical_resolution(disp);
}

void displayTestImage()
{
    inkplate.clearDisplay();
    currentVCOM = readPanelVCOM();

    // Top text: "Current VCOM: <value> V"
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Current VCOM: ");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, 5, 5);

    lv_obj_t *label2 = lv_label_create(lv_scr_act());
    String vcomText = String(currentVCOM, 2) + " V";
    lv_label_set_text(label2, vcomText.c_str());
    lv_obj_set_style_text_color(label2, lv_color_hex(0x000000), 0);
    lv_obj_set_style_text_font(label2, &lv_font_montserrat_28, 0);
    lv_obj_align_to(label2, label, LV_ALIGN_OUT_RIGHT_MID, 5, 0);

    // Grayscale gradient bars across the screen
    for (int i = 0; i < 8; i++)
    {
        int x = (display_width() / 8) * i;

        uint8_t v = (i * 255) / 7;
        lv_color_t color = lv_color_make(v, v, v);

        lv_obj_t *rect = lv_obj_create(lv_scr_act());
        lv_obj_set_size(rect, display_width() / 8, display_height());
        lv_obj_set_pos(rect, x, 40);
        lv_obj_set_style_bg_color(rect, color, 0);
        lv_obj_set_style_border_width(rect, 0, 0);
        lv_obj_set_style_radius(rect, 0, 0);
    }

    // Push LVGL buffer to the e-paper
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}
