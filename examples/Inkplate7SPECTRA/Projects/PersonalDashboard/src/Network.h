#ifndef NETWORK_H
#define NETWORK_H

#include <Arduino.h>
#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include "config.h"

// Maximum number of calendar events to fetch
#define MAX_EVENTS 6

struct WeatherInfo
{
    float  tempC;
    int    weatherCode;     // WMO weather code
    String sunrise;         // "HH:MM"  (used by Day Cycle arc)
    String sunset;          // "HH:MM"  (used by Day Cycle arc)
    float  windSpeed  = 0;  // km/h
    float  precipitation = 0; // mm
    bool   valid = false;
};

struct CalendarEvent
{
    String title;
    String startTime; // "HH:MM" for timed events, "DD.MM." for all-day
    String dayLabel;  // "Today" | "Tomorrow" | "Monday"…"Sunday" | "DD.MM."
    String startISO;  // raw ISO datetime — used for isCurrentEvent check
    String endTime;   // raw ISO datetime/date — used for isCurrentEvent check
};

struct StoicQuote
{
    String text;
    String author;
    bool   valid = false;
};

struct PokemonInfo
{
    String   name;
    String   type;              // primary type (capitalized)
    String   type2;             // secondary type (empty if single-type)
    int      hp      = 0;
    int      attack  = 0;
    int      defense = 0;
    int      speed   = 0;
    int      weight  = 0;       // hectograms → divide by 10 for kg
    int      height  = 0;       // decimeters → divide by 10 for metres
    String   ability;           // first non-hidden ability (capitalized)
    String   weaknesses[10];    // types dealing 2× damage (from type API)
    int      weaknessCount = 0;
    String   description;       // English Pokedex flavor text (from species endpoint)
    uint8_t *spriteData = nullptr;
    size_t   spriteLen  = 0;
    bool     valid = false;
};

class DataFetcher
{
  public:
    // Connect to WiFi, returns true on success (30s timeout)
    bool connectWiFi(const char *ssid, const char *pass);

    // Fetch current weather from OpenMeteo
    bool fetchWeather(WeatherInfo &out);

    // Fetch upcoming calendar events (public Google Calendar)
    bool fetchCalendar(CalendarEvent events[], int &count, int maxResults = MAX_EVENTS);

    // Fetch Pokemon data from PokeAPI for the ID defined by POKEMON_ID in config.h
    bool fetchPokemon(PokemonInfo &out);

    // Fetch a random stoic quote
    bool fetchQuote(StoicQuote &out);

  private:
    String httpGet(const String &url);
    bool   httpGetBinary(const String &url, uint8_t *&outData, size_t &outLen);
    String formatTimeFromISO(const String &iso);  // "2025-01-12T14:30:00+02:00" → "14:30"
    String formatDateFromISO(const String &iso);  // "2025-01-12" → "12.01."
    String dayLabelFromDate(const String &dateStr); // "YYYY-MM-DD" → "Today"/"Tomorrow"/weekday/"DD.MM."
};

#endif
