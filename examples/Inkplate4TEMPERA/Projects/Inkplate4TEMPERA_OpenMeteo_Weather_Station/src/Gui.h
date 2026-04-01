#ifndef GUI_H
#define GUI_H

#include <Inkplate-LVGL.h>
#include "Network.h"
#include "WeatherData.h"
#include <Arduino.h>

class Gui
{
public:
    Gui(Inkplate &inkplate);

    void drawBackground();
    void displayWeatherData(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);
    void wifiError();
    void apiError();

    int batteryLevel;
    double voltage;

private:
    Inkplate &inkplate;

    lv_obj_t *screen = nullptr;

    // Main sections
    lv_obj_t *panelMain = nullptr;
    lv_obj_t *panelGraph = nullptr;
    lv_obj_t *panelWeekly = nullptr;

    // Main info
    lv_obj_t *imgCity = nullptr;
    lv_obj_t *imgTemp = nullptr;
    lv_obj_t *imgWeather = nullptr;
    lv_obj_t *lblCity = nullptr;
    lv_obj_t *lblCurrentTemp = nullptr;
    lv_obj_t *lblWeatherDesc = nullptr;

    // User / battery info
    lv_obj_t *imgBattery = nullptr;
    lv_obj_t *lblBattery = nullptr;
    lv_obj_t *lblLastUpdatedDate = nullptr;
    lv_obj_t *lblLastUpdatedTime = nullptr;
    lv_obj_t *lblUsername = nullptr;

    // Weekly forecast
    static const int WEEKLY_ITEMS = 5;
    lv_obj_t *lblDay[WEEKLY_ITEMS] = {nullptr};
    lv_obj_t *imgDayIcon[WEEKLY_ITEMS] = {nullptr};
    lv_obj_t *lblDayMax[WEEKLY_ITEMS] = {nullptr};
    lv_obj_t *lblDayMin[WEEKLY_ITEMS] = {nullptr};

    void drawTemperaturePrecipGraph(WeatherData *weatherData, NetworkFunctions::UserInfo *userInfo);
    const lv_image_dsc_t *getWeatherIcon(int code);
    const lv_image_dsc_t *getBatteryIcon(int percentage);
    int voltageToPercentage(double voltage);

    void ensureUiCreated();
    void clearScreenObjects();
    void finalizeRender(bool fullRefresh = true);
    lv_obj_t *createText(lv_obj_t *parent, int x, int y, int w, const char *txt, const lv_font_t *font, lv_text_align_t align = LV_TEXT_ALIGN_LEFT, lv_color_t color = lv_color_black());
};

#endif