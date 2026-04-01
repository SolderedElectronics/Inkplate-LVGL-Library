/**
 **************************************************
 * @file        Inkplate4TEMPERA_OpenMeteo_Weather_Station.ino
 * @brief       Open-Meteo weather station dashboard in 3-bit grayscale with
 *              periodic deep-sleep updates on Inkplate 4 TEMPERA.
 *
 * @details     This example connects Inkplate 4 TEMPERA to WiFi, sets local time
 *              using NTP (based on the configured UTC offset), fetches current
 *              weather data from the Open-Meteo API for the specified latitude
 *              and longitude, and renders a dashboard-style screen using a GUI
 *              helper layer (Gui / WeatherData / NetworkFunctions).
 *
 *              After drawing the UI, the ESP32 enters deep sleep for a fixed
 *              interval (default: 30 minutes). On wake-up, the ESP32 restarts
 *              and the sketch runs setup() again, updating the screen with fresh
 *              weather data. The UI also displays user-facing info such as a
 *              username/city label and battery voltage (read from the device).
 *
 * Requirements:
 * - Board:      Soldered Inkplate 4 TEMPERA
 * - Hardware:   Inkplate 4 TEMPERA, USB-C cable
 * - Extra:      WiFi (2.4 GHz), internet access
 *
 * Configuration:
 * - Boards Manager -> Inkplate Boards -> Soldered Inkplate 4 TEMPERA
 * - Serial Monitor: 115200 baud (optional, for debugging)
 * - WiFi credentials / API keys / timezone (if relevant)
 *   - Set ssid / password for your WiFi.
 *   - Set timeZone as UTC offset (e.g., 2 for UTC+2).
 *   - Set latitude / longitude for your location.
 *   - Optional: set myUsername / myCity (display only).
 *   - Units: set metricUnits=false to use Imperial units (as supported by the
 *     helper layer).
 * - NTP server (optional): change ntpServer if you need a different pool.
 *
 * Don't have Inkplate Boards in Arduino Boards Manager?
 * See https://docs.soldered.com/inkplate/10/quick-start-guide/
 *
 * How to use:
 * 1) Enter your WiFi SSID/password and set timeZone, latitude, and longitude.
 * 2) (Optional) Set myUsername and myCity for the on-screen header.
 * 3) Upload the sketch.
 * 4) The device connects to WiFi, fetches weather data, draws the dashboard,
 *    then sleeps. It wakes and refreshes automatically every TIME_TO_SLEEP.
 *
 * Expected output:
 * - E-paper: Weather dashboard UI (icons/text/values as implemented by Gui),
 *   including city/user label and battery voltage.
 * - Error screens: WiFi error screen if connection fails; API error screen if
 *   Open-Meteo fetch/parsing fails.
 *
 * Notes:
 * - Display mode is 3-bit grayscale (INKPLATE_3BIT, 8 levels). Partial update is
 *   not available in grayscale mode; the UI is updated using full refresh.
 * - Deep sleep restarts the ESP32. All runtime state is lost unless stored in
 *   RTC memory; this sketch recomputes everything each wake cycle.
 * - WiFi connection attempts are time-limited; if your network is slow to join,
 *   increase the timeout or retry settings.
 * - RAM usage depends on the UI assets/fonts used by src/includes.h and the GUI
 *   layer; keep additional buffers small if extending the example.
 *
 * Docs:         https://docs.soldered.com/inkplate
 * Support:      https://forum.soldered.com/
 *
 * @author      Soldered
 * @date        2025
 * @license     GNU GPL V3
 **************************************************/

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include "src/includes.h"

// --- WiFi Configuration ---
const char *ssid = "Soldered Electronics";
const char *password = "dasduino";

// --- User and Location Info ---
String myUsername = "Username";
String myCity = "Osijek";
int timeZone = 2;
float latitude = 45.5550;
float longitude = 18.6955;

bool metricUnits = true;

const char *ntpServer = "pool.ntp.org";

// --- Device and Data Objects ---
Inkplate inkplate(INKPLATE_3BIT);
NetworkFunctions network;
NetworkFunctions::UserInfo userInfo;
WeatherData weatherData;
Gui gui(inkplate);

// --- Deep Sleep Configuration ---
#define uS_TO_S_FACTOR 1000000
#define TIME_TO_SLEEP 1800

void setup()
{
    Serial.begin(115200);
    delay(200);

    // Start Inkplate in LVGL mode
    inkplate.begin(LV_DISPLAY_RENDER_MODE_FULL);
    inkplate.selectDisplayMode(INKPLATE_3BIT);
    inkplate.setRotation(0);

    // Attempt to connect to WiFi
    const unsigned long timeout = 30000;
    unsigned long startTime = millis();
    bool connected = false;

    while (!connected && (millis() - startTime) < timeout)
    {
        connected = inkplate.connectWiFi(ssid, password, 10, true);
    }

    if (!connected)
    {
        gui.wifiError();
    }
    else
    {
        configTime(timeZone * 3600, 0, ntpServer);

        gui.voltage = inkplate.battery.voltage();
        userInfo.city = myCity;
        userInfo.username = myUsername;
        userInfo.useMetric = metricUnits;

        network.fetchWeatherData(&weatherData, &userInfo, &latitude, &longitude);

        if (userInfo.apiError)
        {
            gui.apiError();
        }
        else
        {
            gui.drawBackground();
            gui.displayWeatherData(&weatherData, &userInfo);
        }
    }

    esp_sleep_enable_timer_wakeup((uint64_t)TIME_TO_SLEEP * uS_TO_S_FACTOR);
    esp_deep_sleep_start();
}

void loop()
{
    // Nothing to do here - main logic runs once in setup()
}