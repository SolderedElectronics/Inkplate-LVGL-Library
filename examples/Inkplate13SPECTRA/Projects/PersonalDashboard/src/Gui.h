#ifndef GUI_H
#define GUI_H

#include <Inkplate-LVGL.h>
#include "Network.h"

class Gui
{
  public:
    void init();
    void showConnecting();
    void showWifiError();
    void render(WeatherInfo &weather, CalendarEvent events[], int eventCount,
                PokemonInfo &pokemon, StoicQuote &quote);

  private:
    // Panel containers
    lv_obj_t *_pokemonImgPanel = nullptr;
    lv_obj_t *_pokemonPanel    = nullptr;
    lv_obj_t *_weatherPanel    = nullptr;
    lv_obj_t *_dayCyclePanel   = nullptr;
    lv_obj_t *_calendarPanel   = nullptr;
    lv_obj_t *_quotePanel      = nullptr;
    lv_obj_t *_statusLabel     = nullptr;

    // Helpers
    static lv_obj_t *createCard(lv_obj_t *parent, int x, int y, int w, int h,
                                lv_color_t bg, int radius = 16);
    static lv_obj_t *createLabel(lv_obj_t *parent, const char *text,
                                 const lv_font_t *font, lv_color_t color,
                                 int x, int y);

    void renderWeather(WeatherInfo &w);
    void renderDayCycle(WeatherInfo &w);
    void renderCalendar(CalendarEvent events[], int count);
    void renderPokemonSprite(PokemonInfo &p);
    void renderPokemonInfo(PokemonInfo &p);
    void renderQuote(StoicQuote &q);

    // WMO code → human description + accent color
    static const char *wmoDescription(int code);
    static lv_color_t  wmoColor(int code);

    // Pokemon type name → official type color
    static lv_color_t typeColor(const String &type);

    // Calendar day label → dark badge color
    static lv_color_t dayColor(const String &label);

    // Parse "HH:MM" → total minutes since midnight
    static int timeToMinutes(const String &hhmm);

    // Returns true if the current wall-clock time falls between startISO and endISO
    static bool isCurrentEvent(const String &startISO, const String &endISO);
};

#endif
