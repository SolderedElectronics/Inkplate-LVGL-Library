/*
   Inkplate4TEMPERA_BME688_Read_LVGL example for Soldered Inkplate 4 TEMPERA

   This version uses .h image buffers, wrapped manually as lv_image_dsc_t,
   similar to the ImageFromBuffer.ino example.
*/

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <math.h>

#include "pressure.h"
#include "temperature.h"
#include "humidity.h"

// The sensor needs to be calibrated only with a single temperature offset in degrees Celsius
const float offset = -4.0f;

// Variable that keeps count on how many times the screen has been partially updated
int n = 0;

// Inkplate object in 1-bit mode
Inkplate inkplate(INKPLATE_1BIT);

/*
   Image descriptors

   IMPORTANT:
   This assumes your headers define:
   - pressure[] + PRESSURE_WIDTH + PRESSURE_HEIGHT
   - temperature[] + TEMPERATURE_WIDTH + TEMPERATURE_HEIGHT
   - humidity[] + HUMIDITY_WIDTH + HUMIDITY_HEIGHT

   If your converter generated different names, change them here.
*/

// If your images were exported as 8-bit grayscale, use LV_COLOR_FORMAT_L8
const lv_image_dsc_t pressure_img = {
    .header = {
        .cf = LV_COLOR_FORMAT_L8,
        .w  = PRESSURE_WIDTH,
        .h  = PRESSURE_HEIGHT,
    },
    .data_size = PRESSURE_WIDTH * PRESSURE_HEIGHT,
    .data      = pressure,
};

const lv_image_dsc_t temperature_img = {
    .header = {
        .cf = LV_COLOR_FORMAT_L8,
        .w  = TEMPERATURE_WIDTH,
        .h  = TEMPERATURE_HEIGHT,
    },
    .data_size = TEMPERATURE_WIDTH * TEMPERATURE_HEIGHT,
    .data      = temperature,
};

const lv_image_dsc_t humidity_img = {
    .header = {
        .cf = LV_COLOR_FORMAT_L8,
        .w  = HUMIDITY_WIDTH,
        .h  = HUMIDITY_HEIGHT,
    },
    .data_size = HUMIDITY_WIDTH * HUMIDITY_HEIGHT,
    .data      = humidity,
};

// UI objects
static lv_obj_t *img_temp = nullptr;
static lv_obj_t *img_hum = nullptr;
static lv_obj_t *img_press = nullptr;

static lv_obj_t *lbl_temp_title = nullptr;
static lv_obj_t *lbl_hum_title = nullptr;
static lv_obj_t *lbl_press_title = nullptr;

static lv_obj_t *lbl_temp_value = nullptr;
static lv_obj_t *lbl_hum_value = nullptr;
static lv_obj_t *lbl_press_value = nullptr;

static lv_obj_t *lbl_gas_title = nullptr;
static lv_obj_t *lbl_gas_value = nullptr;
static lv_obj_t *lbl_alt_title = nullptr;
static lv_obj_t *lbl_alt_value = nullptr;

static lv_obj_t *lbl_error = nullptr;

// Last shown values so screen only updates when needed
float lastTemperature = NAN;
float lastHumidity = NAN;
float lastPressure = NAN;
float lastGasResistance = NAN;
float lastAltitude = NAN;

void refreshLVGL()
{
    lv_tick_inc(20);
    lv_timer_handler();
}

void createUI()
{
    lv_obj_clean(lv_screen_active());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_black(), LV_PART_MAIN);

    // Temperature icon
    img_temp = lv_image_create(lv_screen_active());
    lv_image_set_src(img_temp, &temperature_img);
    lv_obj_set_pos(img_temp, 93, 100);

    // Humidity icon
    img_hum = lv_image_create(lv_screen_active());
    lv_image_set_src(img_hum, &humidity_img);
    lv_obj_set_pos(img_hum, 378, 100);

    // Pressure icon
    img_press = lv_image_create(lv_screen_active());
    lv_image_set_src(img_press, &pressure_img);
    lv_obj_set_pos(img_press, 93, 368);

    // Titles
    lbl_temp_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_temp_title, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_temp_title, "Temperature:");
    lv_obj_set_pos(lbl_temp_title, 68, 69);

    lbl_hum_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_hum_title, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_hum_title, "Humidity:");
    lv_obj_set_pos(lbl_hum_title, 378, 69);

    lbl_press_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_press_title, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_press_title, "Pressure:");
    lv_obj_set_pos(lbl_press_title, 89, 337);

    lbl_gas_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_gas_title, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_gas_title, "Gas resistance:");
    lv_obj_set_pos(lbl_gas_title, 312, 342);

    lbl_alt_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_alt_title, &lv_font_montserrat_20, 0);
    lv_label_set_text(lbl_alt_title, "Altitude:");
    lv_obj_set_pos(lbl_alt_title, 312, 450);

    // Values
    lbl_temp_value = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_temp_value, &lv_font_montserrat_28, 0);
    lv_label_set_text(lbl_temp_value, "--.-- C");
    lv_obj_set_pos(lbl_temp_value, 100, 241);

    lbl_hum_value = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_hum_value, &lv_font_montserrat_28, 0);
    lv_label_set_text(lbl_hum_value, "--.-- %");
    lv_obj_set_pos(lbl_hum_value, 386, 241);

    lbl_press_value = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_press_value, &lv_font_montserrat_26, 0);
    lv_label_set_text(lbl_press_value, "----.-- hPa");
    lv_obj_set_pos(lbl_press_value, 85, 509);

    lbl_gas_value = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_gas_value, &lv_font_montserrat_22, 0);
    lv_label_set_text(lbl_gas_value, "----.-- mOhm");
    lv_obj_set_pos(lbl_gas_value, 312, 393);

    lbl_alt_value = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_alt_value, &lv_font_montserrat_22, 0);
    lv_label_set_text(lbl_alt_value, "---.-- m");
    lv_obj_set_pos(lbl_alt_value, 312, 501);

    lbl_error = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_error, &lv_font_montserrat_18, 0);
    lv_label_set_text(lbl_error, "");
    lv_obj_set_pos(lbl_error, 20, 570);
}

bool updateSensorLabels(float temperatureValue, float humidityValue, float pressureValue, float gasResistanceValue, float altitudeValue)
{
    bool changed = false;
    char buf[64];

    if (isnan(lastTemperature) || temperatureValue != lastTemperature)
    {
        snprintf(buf, sizeof(buf), "%.2f C", temperatureValue);
        lv_label_set_text(lbl_temp_value, buf);
        lastTemperature = temperatureValue;
        changed = true;
    }

    if (isnan(lastHumidity) || humidityValue != lastHumidity)
    {
        snprintf(buf, sizeof(buf), "%.2f %%", humidityValue);
        lv_label_set_text(lbl_hum_value, buf);
        lastHumidity = humidityValue;
        changed = true;
    }

    if (isnan(lastPressure) || pressureValue != lastPressure)
    {
        snprintf(buf, sizeof(buf), "%.2f hPa", pressureValue);
        lv_label_set_text(lbl_press_value, buf);
        lastPressure = pressureValue;
        changed = true;
    }

    if (isnan(lastGasResistance) || gasResistanceValue != lastGasResistance)
    {
        snprintf(buf, sizeof(buf), "%.2f mOhm", gasResistanceValue);
        lv_label_set_text(lbl_gas_value, buf);
        lastGasResistance = gasResistanceValue;
        changed = true;
    }

    if (isnan(lastAltitude) || altitudeValue != lastAltitude)
    {
        snprintf(buf, sizeof(buf), "%.2f m", altitudeValue);
        lv_label_set_text(lbl_alt_value, buf);
        lastAltitude = altitudeValue;
        changed = true;
    }

    return changed;
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);

    // Useful when showing grayscale images on BW e-paper
    inkplate.enableDithering(1);

    createUI();

    // Enable the BME688 sensor
    inkplate.wakePeripheral(INKPLATE_BME688);

    if (!inkplate.bme688.begin())
    {
        lv_label_set_text(lbl_error, "Can't init BME688!");
        refreshLVGL();
        inkplate.display();
        while (true)
            ;
    }

    refreshLVGL();
    inkplate.display();
}

void loop()
{
    float temperatureValue = inkplate.bme688.readTemperature() + offset;
    float humidityValue = inkplate.bme688.readHumidity();
    float pressureValue = inkplate.bme688.readPressure();
    float gasResistanceValue = inkplate.bme688.readGasResistance();
    float altitudeValue = inkplate.bme688.readAltitude();

    // Uncomment this if pressure is returned in Pa instead of hPa
    // pressureValue /= 100.0f;

    bool dataChanged = updateSensorLabels(
        temperatureValue,
        humidityValue,
        pressureValue,
        gasResistanceValue,
        altitudeValue
    );

    if (dataChanged)
    {
        refreshLVGL();

        if (n > 9)
        {
            inkplate.display();
            n = 0;
        }
        else
        {
            inkplate.partialUpdate(false, true);
            n++;
        }
    }

    delay(1000);
}