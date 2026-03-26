/**
 **************************************************
 *
 * @file        PersonalDashboard.ino
 * @brief       A personal dashboard for Inkplate 13 SPECTRA showing live weather,
 *              Google Calendar events, a random Pokemon, and a day/night cycle arc.
 *
 * SETUP:
 *  1. Open src/config.h and fill in your WiFi credentials, Google Calendar ID,
 *     Google API key, and your location coordinates.
 *  2. Make sure your Google Calendar is set to PUBLIC and the Calendar API
 *     is enabled at console.cloud.google.com.
 *  3. Install the ArduinoJson library (Tools > Manage Libraries > ArduinoJson).
 *  4. Select "Inkplate 13 (ESP32-S3)" in the boards menu and upload.
 *
 * The board wakes from deep sleep every 30 minutes and refreshes all data.
 *
 * For info on how to quickly get started with Inkplate 13 SPECTRA visit
 * https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

#include "src/includes.h"

// Deep sleep refresh interval
#define SLEEP_SECONDS 1800 // 30 minutes

// Global instances
Inkplate inkplate;
DataFetcher  network;
Gui      gui;

// Data containers
WeatherInfo   weather;
CalendarEvent events[MAX_EVENTS];
int           eventCount = 0;
PokemonInfo   pokemon;
StoicQuote    quote;

void setup()
{
    Serial.begin(115200);

    // Init display
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);

    // Build UI skeleton (not displayed yet — single refresh at the end)
    gui.init();

    // Connect to WiFi
    Serial.println("Connecting to WiFi...");
    if (!network.connectWiFi(WIFI_SSID, WIFI_PASS))
    {
        Serial.println("WiFi failed!");
        gui.showWifiError();
        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();

        // Retry after 10 minutes instead of full 30
        esp_sleep_enable_timer_wakeup(600ULL * 1000000ULL);
        esp_deep_sleep_start();
    }
    Serial.println("WiFi connected.");

    // Sync time via NTP
    configTime(TIMEZONE * 3600, 0, "pool.ntp.org");
    delay(2000); // give NTP a moment

    // Fetch all data
    Serial.println("Fetching weather...");
    network.fetchWeather(weather);

    Serial.println("Fetching calendar...");
    network.fetchCalendar(events, eventCount);

    Serial.println("Fetching Pokemon...");
    network.fetchPokemon(pokemon);

    Serial.println("Fetching quote...");
    network.fetchQuote(quote);

    // Render dashboard
    Serial.println("Rendering...");
    gui.render(weather, events, eventCount, pokemon, quote);
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();

    Serial.printf("Done. Sleeping for %d minutes.\n", SLEEP_SECONDS / 60);

    // Deep sleep until next refresh
    esp_sleep_enable_timer_wakeup(SLEEP_SECONDS * 1000000ULL);
    esp_deep_sleep_start();
}

void loop()
{
    // Never reached — all logic is in setup() due to deep sleep
}
