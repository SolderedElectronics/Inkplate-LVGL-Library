/**
 **************************************************
 *
 * @file        Qwiic.ino
 * @brief       Example demonstrating easyC/Qwiic (I2C) communication between Inkplate 5V2 and 
 *              a Soldered BME280 or BME680 environmental sensor, using the LVGL library 
 *              for display rendering. 
 *
 *              This example reads temperature, humidity, and pressure data from a 
 *              Soldered BME280 sensor connected via the easyC interface and displays 
 *              it on the Inkplate 5V2 e-paper screen using LVGL elements.
 *
 *              Note: Both BME280 and BME680 sensors are supported by the same 
 *              Soldered BME280/BME680 library. In this example, the BME280 is used, 
 *              as defined in the code. The sensor communicates over easyC (I2C) 
 *              using the default 0x76 address.
 *
 * For setup instructions and more information about Inkplate 5V2, visit:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * @hardware    Inkplate 5V2 (ESP32-based e-paper display)
 * @sensors     Soldered BME280 (or BME680) via easyC connector
 * @library     Soldered BME280/BME680 Gas Sensor Arduino Library
 *              https://github.com/SolderedElectronics/Soldered-BME280-BME680-Gas-Sensor-Arduino-Library
 *
 * @authors     Soldered
 * @date        November 2025
 **************************************************
 */


#ifndef ARDUINO_INKPLATE5V2
#error "Wrong board selection for this example, please select Soldered Inkplate5 V2 in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <BME280-SOLDERED.h>   // Soldered BME280/BME680 library (easyC/I2C)

// Inkplate in 1-bit mode (fast partial updates)
Inkplate inkplate(INKPLATE_1BIT);
BME280   bme280;               // easyC/I2C (Soldered uses 0x76)

// Optional temperature calibration offset
const float TEMPERATURE_OFFSET = 0.0f;

// UI elements
static lv_obj_t *panel_data;
static lv_obj_t *lbl_title;
static lv_obj_t *lbl_temp;
static lv_obj_t *lbl_hum;
static lv_obj_t *lbl_press;
static lv_obj_t *lbl_warn;

// Refresh policy (do a FULL every ~20 partials)
static int partialCount = 0;
static const int FULL_REFRESH_EVERY = 20;

// Helpers
static inline int dispW() { auto *d = lv_display_get_default(); return lv_display_get_horizontal_resolution(d); }
static inline int dispH() { auto *d = lv_display_get_default(); return lv_display_get_vertical_resolution(d); }

static void make_opaque(lv_obj_t *obj) {
  lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
}

// Minimal LVGL UI (no GFX icons to keep it LVGL-only)
static void ui_create() {
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  lbl_title = lv_label_create(lv_screen_active());
  lv_label_set_text(lbl_title, "easyC • BME280");
  lv_obj_set_style_text_color(lbl_title, lv_color_hex(0x000000), 0);
  lv_obj_set_pos(lbl_title, 8, 8);
  make_opaque(lbl_title);

  panel_data = lv_obj_create(lv_screen_active());
  make_opaque(panel_data);
  lv_obj_set_style_pad_all(panel_data, 0, 0);
  lv_obj_set_style_border_width(panel_data, 0, 0);
  lv_obj_set_pos(panel_data, 0, 40);
  lv_obj_set_size(panel_data, dispW(), dispH() - 80);

  lbl_temp = lv_label_create(panel_data);
  lv_obj_set_style_text_color(lbl_temp, lv_color_hex(0x000000), 0);
  lv_obj_set_pos(lbl_temp, 24, 40);

  lbl_hum = lv_label_create(panel_data);
  lv_obj_set_style_text_color(lbl_hum, lv_color_hex(0x000000), 0);
  lv_obj_set_pos(lbl_hum, 24, 140);

  lbl_press = lv_label_create(panel_data);
  lv_obj_set_style_text_color(lbl_press, lv_color_hex(0x000000), 0);
  lv_obj_set_pos(lbl_press, 24, 240);

  lbl_warn = lv_label_create(panel_data); // shown only if sensor looks unresponsive
  lv_obj_set_style_text_color(lbl_warn, lv_color_hex(0x000000), 0);
  lv_obj_set_pos(lbl_warn, 24, 320);
  lv_label_set_text(lbl_warn, "");
}

static void update_readings() {
  // Read sensor
  float tC  = bme280.readTemperature() + TEMPERATURE_OFFSET; // °C
  float hum = bme280.readHumidity() / 10.0f;                 // library returns x10
  float hPa = bme280.readPressure() * 10.0f;                 // hPa (original style)

  // Basic sanity check (some libs return 0 or extreme values if not connected)
  bool looks_bad = (hPa <= 0.0f) || (hum < 0.0f || hum > 100.0f) || (tC < -40.0f || tC > 85.0f);

  char bufT[32], bufH[32], bufP[32];
  snprintf(bufT, sizeof(bufT), "Temperature: %.1f *C", tC);
  snprintf(bufH, sizeof(bufH), "Humidity:    %.0f %%", hum);
  snprintf(bufP, sizeof(bufP), "Pressure:    %.0f hPa", hPa);

  lv_label_set_text(lbl_temp,  bufT);
  lv_label_set_text(lbl_hum,   bufH);
  lv_label_set_text(lbl_press, bufP);

  if (looks_bad) {
    lv_label_set_text(lbl_warn, "Warning: sensor readings look invalid.\nCheck easyC cable and sensor power.");
  } else {
    lv_label_set_text(lbl_warn, "");
  }
}

void setup() {
  Serial.begin(115200);

  // LVGL + Inkplate in PARTIAL mode
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
  inkplate.selectDisplayMode(INKPLATE_1BIT);

  // Init BME280 (void return in this library)
  bme280.begin();

  ui_create();
  update_readings();

  // Initial FULL refresh
  lv_tick_inc(20);
  lv_timer_handler();
  inkplate.display();
}

void loop() {
  update_readings();

  lv_tick_inc(10);
  lv_timer_handler();

  if (partialCount >= FULL_REFRESH_EVERY) {
    inkplate.display();   // FULL
    partialCount = 0;
  } else {
    inkplate.partialUpdate();
    partialCount++;
  }

  delay(10000); // ~10 s between readings
}
