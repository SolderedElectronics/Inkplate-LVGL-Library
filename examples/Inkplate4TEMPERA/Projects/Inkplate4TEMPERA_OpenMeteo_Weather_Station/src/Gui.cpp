#include "Gui.h"
#include "Network.h"
#include "WeatherData.h"
#include <Arduino.h>
#include <Inkplate-LVGL.h>

// all the weather icons
#include "binary_Icons/icon_s_clear_sky.h"
#include "binary_Icons/icon_s_fog.h"
#include "binary_Icons/icon_s_gray.h"
#include "binary_Icons/icon_s_moon.h"
#include "binary_Icons/icon_s_partly_cloudy.h"
#include "binary_Icons/icon_s_rain.h"
#include "binary_Icons/icon_s_snow.h"
#include "binary_Icons/icon_s_storm.h"
#include "binary_Icons/icon_s_thermometer.h"

// all the battery icons
#include "binary_Icons/icon_s_full_battery.h"
#include "binary_Icons/icon_s_half_battery.h"
#include "binary_Icons/icon_s_high_battery.h"
#include "binary_Icons/icon_s_low_battery.h"

namespace
{
    static constexpr uint32_t ICON_W = 48;
    static constexpr uint32_t ICON_H = 48;
    static constexpr uint32_t ICON_DATA_SIZE = ICON_W * ICON_H; // 1 byte per pixel

    // These icon buffers are stored as 8-bit grayscale, not packed 1-bit.
    const lv_image_dsc_t img_icon_s_clear_sky = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_clear_sky,
    };

    const lv_image_dsc_t img_icon_s_fog = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_fog,
    };

    const lv_image_dsc_t img_icon_s_gray = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_gray,
    };

    const lv_image_dsc_t img_icon_s_moon = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_moon,
    };

    const lv_image_dsc_t img_icon_s_partly_cloudy = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_partly_cloudy,
    };

    const lv_image_dsc_t img_icon_s_rain = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_rain,
    };

    const lv_image_dsc_t img_icon_s_snow = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_snow,
    };

    const lv_image_dsc_t img_icon_s_storm = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_storm,
    };

    const lv_image_dsc_t img_icon_s_thermometer = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_thermometer,
    };

    const lv_image_dsc_t img_icon_s_full_battery = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_full_battery,
    };

    const lv_image_dsc_t img_icon_s_half_battery = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_half_battery,
    };

    const lv_image_dsc_t img_icon_s_high_battery = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_high_battery,
    };

    const lv_image_dsc_t img_icon_s_low_battery = {
        .header = {.cf = LV_COLOR_FORMAT_L8, .w = ICON_W, .h = ICON_H},
        .data_size = ICON_DATA_SIZE,
        .data = icon_s_low_battery,
    };
}

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate)
{
}

lv_obj_t *Gui::createText(lv_obj_t *parent, int x, int y, int w, const char *txt, const lv_font_t *font, lv_text_align_t align, lv_color_t color)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_obj_set_pos(label, x, y);
    lv_obj_set_width(label, w);
    lv_label_set_text(label, txt);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_align(label, align, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);
    return label;
}

void Gui::clearScreenObjects()
{
    screen = lv_scr_act();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    panelMain = nullptr;
    panelGraph = nullptr;
    panelWeekly = nullptr;

    imgCity = nullptr;
    imgTemp = nullptr;
    imgWeather = nullptr;
    lblCity = nullptr;
    lblCurrentTemp = nullptr;
    lblWeatherDesc = nullptr;

    imgBattery = nullptr;
    lblBattery = nullptr;
    lblLastUpdatedDate = nullptr;
    lblLastUpdatedTime = nullptr;
    lblUsername = nullptr;

    for (int i = 0; i < WEEKLY_ITEMS; i++)
    {
        lblDay[i] = nullptr;
        imgDayIcon[i] = nullptr;
        lblDayMax[i] = nullptr;
        lblDayMin[i] = nullptr;
    }
}

void Gui::ensureUiCreated()
{
    if (screen != nullptr && panelMain != nullptr && panelGraph != nullptr && panelWeekly != nullptr)
        return;

    clearScreenObjects();

    panelMain = lv_obj_create(screen);
    lv_obj_remove_style_all(panelMain);
    lv_obj_set_pos(panelMain, 0, 0);
    lv_obj_set_size(panelMain, 600, 160);
    lv_obj_set_style_bg_color(panelMain, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(panelMain, LV_OPA_COVER, 0);

    panelGraph = lv_obj_create(screen);
    lv_obj_remove_style_all(panelGraph);
    lv_obj_set_pos(panelGraph, 0, 160);
    lv_obj_set_size(panelGraph, 600, 290);
    lv_obj_set_style_bg_color(panelGraph, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(panelGraph, LV_OPA_COVER, 0);

    panelWeekly = lv_obj_create(screen);
    lv_obj_remove_style_all(panelWeekly);
    lv_obj_set_pos(panelWeekly, 0, 450);
    lv_obj_set_size(panelWeekly, 600, 150);
    lv_obj_set_style_bg_color(panelWeekly, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(panelWeekly, LV_OPA_COVER, 0);

    imgCity = lv_image_create(panelMain);
    lv_obj_set_pos(imgCity, 10, 5);
    lv_image_set_src(imgCity, &img_icon_s_gray);

    lblCity = createText(panelMain, 70, 18, 250, "", &lv_font_montserrat_22, LV_TEXT_ALIGN_LEFT, lv_color_white());

    imgTemp = lv_image_create(panelMain);
    lv_obj_set_pos(imgTemp, 10, 55);
    lv_image_set_src(imgTemp, &img_icon_s_thermometer);

    lblCurrentTemp = createText(panelMain, 70, 68, 180, "", &lv_font_montserrat_22, LV_TEXT_ALIGN_LEFT, lv_color_white());

    imgWeather = lv_image_create(panelMain);
    lv_obj_set_pos(imgWeather, 10, 106);

    lblWeatherDesc = createText(panelMain, 70, 116, 250, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, lv_color_white());

    imgBattery = lv_image_create(panelMain);
    lv_obj_set_pos(imgBattery, 390, 10);

    lblBattery = createText(panelMain, 440, 20, 100, "", &lv_font_montserrat_16, LV_TEXT_ALIGN_LEFT, lv_color_white());
    lblLastUpdatedDate = createText(panelMain, 390, 50, 180, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, lv_color_white());
    lblLastUpdatedTime = createText(panelMain, 390, 80, 190, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, lv_color_white());
    lblUsername = createText(panelMain, 390, 110, 180, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_LEFT, lv_color_white());

    int startX = 20;
    int spacing = 112;
    for (int i = 0; i < WEEKLY_ITEMS; i++)
    {
        int xPos = startX + i * spacing;

        lblDay[i] = createText(panelWeekly, xPos, 8, 90, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER, lv_color_white());

        imgDayIcon[i] = lv_image_create(panelWeekly);
        lv_obj_set_pos(imgDayIcon[i], xPos + 21, 30);

        lblDayMax[i] = createText(panelWeekly, xPos, 84, 90, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER, lv_color_white());
        lblDayMin[i] = createText(panelWeekly, xPos, 108, 90, "", &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER, lv_color_white());
    }
}

void Gui::finalizeRender(bool fullRefresh)
{
    for (int i = 0; i < 3; i++)
    {
        lv_timer_handler();
        delay(10);
    }

    if (fullRefresh)
        inkplate.display();
    else
        inkplate.partialUpdate(false, true);
}

void Gui::drawBackground()
{
    ensureUiCreated();
}

void Gui::wifiError()
{
    clearScreenObjects();

    createText(screen, 50, 140, 500, "WiFi connection failed.", &lv_font_montserrat_20);
    createText(screen, 50, 190, 500, "Check credentials or try again.", &lv_font_montserrat_20);

    finalizeRender(true);
}

void Gui::apiError()
{
    clearScreenObjects();

    createText(screen, 50, 140, 500, "HTTP request failed.", &lv_font_montserrat_20);
    createText(screen, 50, 190, 500, "Check API URL or try again.", &lv_font_montserrat_20);

    finalizeRender(true);
}

int Gui::voltageToPercentage(double voltage)
{
    if (voltage >= 4.2) return 100;
    if (voltage <= 3.0) return 0;

    return (int)(((voltage - 3.0) / (4.2 - 3.0)) * 100);
}

const lv_image_dsc_t *Gui::getWeatherIcon(int code)
{
    switch (code)
    {
    case 0:
        return &img_icon_s_clear_sky;
    case 1:
    case 2:
    case 3:
        return &img_icon_s_partly_cloudy;
    case 45:
    case 48:
        return &img_icon_s_fog;
    case 51:
    case 53:
    case 55:
    case 56:
    case 57:
    case 61:
    case 63:
    case 65:
    case 66:
    case 67:
    case 80:
    case 81:
    case 82:
        return &img_icon_s_rain;
    case 71:
    case 73:
    case 75:
    case 77:
    case 85:
    case 86:
        return &img_icon_s_snow;
    case 95:
    case 96:
    case 99:
        return &img_icon_s_storm;
    default:
        return &img_icon_s_gray;
    }
}

const lv_image_dsc_t *Gui::getBatteryIcon(int percentage)
{
    if (percentage >= 75)
        return &img_icon_s_full_battery;
    else if (percentage >= 50)
        return &img_icon_s_high_battery;
    else if (percentage >= 25)
        return &img_icon_s_half_battery;
    else
        return &img_icon_s_low_battery;
}

void Gui::drawTemperaturePrecipGraph(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    if (panelGraph == nullptr) return;

    lv_obj_clean(panelGraph);
    lv_obj_set_style_bg_color(panelGraph, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(panelGraph, LV_OPA_COVER, 0);

    createText(panelGraph, 10, 10, 400, "Hourly temperature and precipitation", &lv_font_montserrat_14);

    int graphX = 60;
    int graphY = 35;
    int graphWidth = 500;
    int graphHeight = 225;

    int marginX = 20;
    int marginY = 20;
    int chartLeft = graphX + marginX;
    int chartBottom = graphY + graphHeight - marginY;
    int chartTop = graphY + marginY;

    float actualTempMin = 100.0f;
    float actualTempMax = -100.0f;
    float precipMax = 100.0f;

    for (int i = 0; i < 6; i++)
    {
        if (weatherData->hourlyTemps[i] < actualTempMin)
            actualTempMin = weatherData->hourlyTemps[i];
        if (weatherData->hourlyTemps[i] > actualTempMax)
            actualTempMax = weatherData->hourlyTemps[i];
    }

    float paddedTempMin = actualTempMin - 2.0f;
    float paddedTempMax = actualTempMax + 2.0f;
    float paddedTempMid = (paddedTempMin + paddedTempMax) / 2.0f;

    float tempRange = paddedTempMax - paddedTempMin;
    if (tempRange == 0.0f)
        tempRange = 1.0f;

    float xStep = (graphWidth - 2 * marginX) / 5.0f;

    lv_obj_t *yAxis = lv_obj_create(panelGraph);
    lv_obj_remove_style_all(yAxis);
    lv_obj_set_pos(yAxis, chartLeft, chartTop);
    lv_obj_set_size(yAxis, 1, chartBottom - chartTop);
    lv_obj_set_style_bg_color(yAxis, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(yAxis, LV_OPA_COVER, 0);

    lv_obj_t *xAxis = lv_obj_create(panelGraph);
    lv_obj_remove_style_all(xAxis);
    lv_obj_set_pos(xAxis, chartLeft, chartBottom);
    lv_obj_set_size(xAxis, graphWidth + 20 - marginX, 1);
    lv_obj_set_style_bg_color(xAxis, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(xAxis, LV_OPA_COVER, 0);

    int yMin = chartBottom - ((paddedTempMin - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY);
    int yMax = chartBottom - ((paddedTempMax - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY);
    int yMid = chartBottom - ((paddedTempMid - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY);

    if (abs(yMax - yMin) < 20)
    {
        int offset = 20 - abs(yMax - yMin);
        yMax = yMin + offset;
    }

    if (abs(yMid - yMin) < 20)
    {
        int offset = 20 - abs(yMid - yMin);
        yMid = yMin + offset;
    }

    if (abs(yMax - yMid) < 20)
    {
        int offset = 20 - abs(yMax - yMid);
        yMax = yMid + offset;
    }

    char buf[32];

    snprintf(buf, sizeof(buf), "%.1f%s", paddedTempMin, userInfo->temperatureLabel.c_str());
    createText(panelGraph, chartLeft - 68, yMin - 24, 64, buf, &lv_font_montserrat_14, LV_TEXT_ALIGN_RIGHT);

    snprintf(buf, sizeof(buf), "%.1f%s", paddedTempMid, userInfo->temperatureLabel.c_str());
    createText(panelGraph, chartLeft - 68, yMid - 10, 64, buf, &lv_font_montserrat_14, LV_TEXT_ALIGN_RIGHT);

    snprintf(buf, sizeof(buf), "%.1f%s", paddedTempMax, userInfo->temperatureLabel.c_str());
    createText(panelGraph, chartLeft - 68, yMax + 8, 64, buf, &lv_font_montserrat_14, LV_TEXT_ALIGN_RIGHT);

    for (int i = 0; i < 6; i++)
    {
        int x = chartLeft + i * xStep;
        int barHeight = (precipMax > 0) ? (int)((weatherData->hourlyPrecip[i] / precipMax) * (graphHeight - 2 * marginY)) : 0;
        int y = chartBottom - barHeight;

        if (barHeight > 10)
        {
            lv_obj_t *bar = lv_obj_create(panelGraph);
            lv_obj_remove_style_all(bar);
            lv_obj_set_pos(bar, x + 5, y + 10);
            lv_obj_set_size(bar, 10, barHeight - 10);
            lv_obj_set_style_bg_color(bar, lv_palette_main(LV_PALETTE_GREY), 0);
            lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
        }

        snprintf(buf, sizeof(buf), "%.0f%%", weatherData->hourlyPrecip[i]);
        createText(panelGraph, x - 10, y - 15, 40, buf, &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);

        createText(panelGraph, x - 10, chartBottom + 8, 40, weatherData->hourlyTimes[i].c_str(), &lv_font_montserrat_14, LV_TEXT_ALIGN_CENTER);
    }

    for (int i = 0; i < 5; i++)
    {
        int x1 = chartLeft + i * xStep;
        int x2 = chartLeft + (i + 1) * xStep;

        int y1 = chartBottom - ((weatherData->hourlyTemps[i] - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY);
        int y2 = chartBottom - ((weatherData->hourlyTemps[i + 1] - paddedTempMin) / tempRange) * (graphHeight - 2 * marginY);

        int dx = x2 - x1;
        int dy = y2 - y1;
        float length = sqrtf((float)(dx * dx + dy * dy));
        float angle = atan2f((float)dy, (float)dx) * 1800.0f / 3.14159265f;

        lv_obj_t *seg = lv_obj_create(panelGraph);
        lv_obj_remove_style_all(seg);
        lv_obj_set_pos(seg, x1, y1);
        lv_obj_set_size(seg, (lv_coord_t)length, 2);
        lv_obj_set_style_bg_color(seg, lv_color_black(), 0);
        lv_obj_set_style_bg_opa(seg, LV_OPA_COVER, 0);
        lv_obj_set_style_transform_pivot_x(seg, 0, 0);
        lv_obj_set_style_transform_pivot_y(seg, 1, 0);
        lv_obj_set_style_transform_rotation(seg, (int16_t)angle, 0);
    }
}

void Gui::displayWeatherData(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo)
{
    ensureUiCreated();

    lv_image_set_src(imgCity, &img_icon_s_gray);
    lv_label_set_text(lblCity, userInfo->city.c_str());

    lv_image_set_src(imgTemp, &img_icon_s_thermometer);

    String tempText = String(weatherData->currentTemp, 1) + userInfo->temperatureLabel;
    lv_label_set_text(lblCurrentTemp, tempText.c_str());

    lv_image_set_src(imgWeather, getWeatherIcon(weatherData->weatherCode));
    lv_label_set_text(lblWeatherDesc, weatherData->weatherDescription.c_str());

    batteryLevel = voltageToPercentage(voltage);
    lv_image_set_src(imgBattery, getBatteryIcon(batteryLevel));

    String batteryText = String(batteryLevel) + "%";
    lv_label_set_text(lblBattery, batteryText.c_str());
    lv_label_set_text(lblLastUpdatedDate, userInfo->lastUpdatedDate.c_str());

    String refreshText = "Last refresh: " + userInfo->lastUpdatedTime;
    lv_label_set_text(lblLastUpdatedTime, refreshText.c_str());
    lv_label_set_text(lblUsername, userInfo->username.c_str());

    for (int i = 0; i < WEEKLY_ITEMS; i++)
    {
        lv_label_set_text(lblDay[i], weatherData->dailyNames[i].c_str());
        lv_image_set_src(imgDayIcon[i], getWeatherIcon(weatherData->dailyWeatherCodes[i]));

        String maxText = "▲ " + String(weatherData->dailyMaxTemp[i], 0) + userInfo->temperatureLabel;
        String minText = "▼ " + String(weatherData->dailyMinTemp[i], 0) + userInfo->temperatureLabel;

        lv_label_set_text(lblDayMax[i], maxText.c_str());
        lv_label_set_text(lblDayMin[i], minText.c_str());
    }

    drawTemperaturePrecipGraph(weatherData, userInfo);
    finalizeRender(true);
}