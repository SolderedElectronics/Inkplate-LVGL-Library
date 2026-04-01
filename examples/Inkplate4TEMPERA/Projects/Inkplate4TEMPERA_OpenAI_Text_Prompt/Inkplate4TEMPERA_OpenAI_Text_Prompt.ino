/**
 **************************************************
 * @file        Inkplate4TEMPERA_OpenAI_Text_Prompt_LVGL.ino
 * @brief       Fetch current weather, generate a witty text via OpenAI API,
 *              display it with LVGL on Inkplate 4 TEMPERA, then deep sleep.
 **************************************************/

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <WiFi.h>
#include <WiFiClientSecure.h>
#include <ArduinoJson.h>
#include "Inkplate-LVGL.h"

// Sleep duration in seconds
#define SLEEP_DURATION_SECONDS (30 * 60)

// WiFi credentials
const char *ssid = "Soldered Electronics";
const char *password = "dasduino";

// OpenAI API
const char *openai_key = "your api key";
const char *openai_model = "gpt-4o-mini";

// Location and coordinates
String location = "Osijek";
const float latitude = 45.5600f;
const float longitude = 18.6750f;

// Weather data
float temperature = 0.0f;
String weatherDesc, timeStr;

// HTTPS clients
WiFiClientSecure weatherClient;
WiFiClientSecure openaiClient;

// Inkplate LVGL display in 1-bit mode
Inkplate inkplate(INKPLATE_1BIT);

// UI objects
static lv_obj_t *screenMain = nullptr;
static lv_obj_t *statusLabel = nullptr;
static lv_obj_t *contentBox = nullptr;
static lv_obj_t *contentLabel = nullptr;

// Forward declarations
void buildUi();
void setStatus(const char *text, bool partialRefresh = true);
void setContent(const char *text, bool fullRefresh = false);
void renderNow(bool fullRefresh);
bool connectToWiFi();
bool getWeather(float lat, float lon, float &temperatureOut, String &weatherDescOut, String &timeStrOut);
String weatherCodeToString(int code);
String getOpenAIResponse(const String &prompt);
String readHttpBodySkippingChunkMarkers(WiFiClientSecure &client);
bool waitForClientData(WiFiClientSecure &client, uint32_t timeoutMs);

void setup()
{
    Serial.begin(115200);
    delay(100);

    inkplate.begin();
    buildUi();

    bool ok = true;

    setStatus("Connecting to WiFi", true);

    if (!connectToWiFi())
    {
        setContent("Failed to connect to WiFi.", true);
        ok = false;
    }

    if (ok)
    {
        setStatus("Fetching weather...", true);

        if (!getWeather(latitude, longitude, temperature, weatherDesc, timeStr))
        {
            setContent("Failed to get weather data.", true);
            ok = false;
        }
    }

    if (ok)
    {
        setStatus("Generating summary...", true);

        String prompt =
            "Give me a sarcastic 65-word max summary of the weather in " + location +
            ". It's currently " + String(temperature, 1) + "C with " + weatherDesc +
            " skies at " + timeStr +
            ". Use only the hour and minutes from the time. "
            "Make it witty and slightly condescending. "
            "Do not use em dashes. Use normal hyphens instead.";

        String snarkySummary = getOpenAIResponse(prompt);

        if (snarkySummary.length() == 0)
        {
            setContent("OpenAI request failed.", true);
            ok = false;
        }
        else
        {
            setStatus("Done", false);
            setContent(snarkySummary.c_str(), true);
        }
    }

    inkplate.rtc.setAlarmEpoch(
        inkplate.rtc.getEpoch() + SLEEP_DURATION_SECONDS,
        RTC_ALARM_MATCH_DHHMMSS
    );

    esp_sleep_enable_ext0_wakeup(GPIO_NUM_39, 0);
    esp_deep_sleep_start();
}

void loop()
{
    // Never reached because of deep sleep
}

void buildUi()
{
    screenMain = lv_obj_create(NULL);
    lv_obj_remove_style_all(screenMain);
    lv_obj_set_style_bg_color(screenMain, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screenMain, LV_OPA_COVER, 0);

    lv_obj_t *title = lv_label_create(screenMain);
    lv_label_set_text(title, "Weather Snark");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 16);

    lv_obj_t *divider = lv_obj_create(screenMain);
    lv_obj_remove_style_all(divider);
    lv_obj_set_size(divider, 540, 3);
    lv_obj_set_style_bg_color(divider, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(divider, LV_OPA_COVER, 0);
    lv_obj_align(divider, LV_ALIGN_TOP_MID, 0, 56);

    statusLabel = lv_label_create(screenMain);
    lv_label_set_text(statusLabel, "Starting...");
    lv_obj_set_style_text_font(statusLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(statusLabel, lv_color_black(), 0);
    lv_obj_align(statusLabel, LV_ALIGN_TOP_LEFT, 28, 72);

    contentBox = lv_obj_create(screenMain);
    lv_obj_remove_style_all(contentBox);
    lv_obj_set_size(contentBox, 540, 450);
    lv_obj_align(contentBox, LV_ALIGN_TOP_MID, 0, 110);
    lv_obj_set_style_border_width(contentBox, 1, 0);
    lv_obj_set_style_border_color(contentBox, lv_color_black(), 0);
    lv_obj_set_style_bg_color(contentBox, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(contentBox, LV_OPA_COVER, 0);
    lv_obj_set_style_radius(contentBox, 0, 0);
    lv_obj_set_style_pad_all(contentBox, 18, 0);
    lv_obj_set_scrollbar_mode(contentBox, LV_SCROLLBAR_MODE_OFF);

    contentLabel = lv_label_create(contentBox);
    lv_obj_set_width(contentLabel, 500);
    lv_label_set_long_mode(contentLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(contentLabel, "Waiting for data...");
    lv_obj_set_style_text_font(contentLabel, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(contentLabel, lv_color_black(), 0);
    lv_obj_align(contentLabel, LV_ALIGN_TOP_LEFT, 0, 0);

    lv_scr_load(screenMain);
    renderNow(true);
}

void setStatus(const char *text, bool partialRefresh)
{
    lv_label_set_text(statusLabel, text);
    renderNow(!partialRefresh ? true : false);
}

void setContent(const char *text, bool fullRefresh)
{
    lv_label_set_text(contentLabel, text);
    renderNow(fullRefresh);
}

void renderNow(bool fullRefresh)
{
    lv_timer_handler();
    delay(15);
    lv_timer_handler();
    delay(15);

    if (fullRefresh)
        inkplate.display();
    else
        inkplate.partialUpdate();
}

bool connectToWiFi()
{
    WiFi.begin(ssid, password);

    String dots = "";
    uint32_t startMs = millis();

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(500);

        if (dots.length() >= 3)
            dots = "";
        else
            dots += ".";

        String msg = "Connecting to WiFi" + dots;
        setStatus(msg.c_str(), true);

        if (millis() - startMs > 30000)
        {
            Serial.println("WiFi timeout");
            return false;
        }
    }

    Serial.println("WiFi connected");
    setStatus("WiFi connected", true);
    return true;
}

bool getWeather(float lat, float lon, float &temperatureOut, String &weatherDescOut, String &timeStrOut)
{
    weatherClient.setInsecure();

    String url =
        "/v1/forecast?latitude=" + String(lat, 4) +
        "&longitude=" + String(lon, 4) +
        "&current_weather=true&timezone=auto";

    if (!weatherClient.connect("api.open-meteo.com", 443))
    {
        Serial.println("Connection to Open-Meteo failed");
        return false;
    }

    weatherClient.println("GET " + url + " HTTP/1.1");
    weatherClient.println("Host: api.open-meteo.com");
    weatherClient.println("Connection: close");
    weatherClient.println();

    if (!waitForClientData(weatherClient, 10000))
    {
        Serial.println("Open-Meteo timeout");
        weatherClient.stop();
        return false;
    }

    while (weatherClient.connected())
    {
        String line = weatherClient.readStringUntil('\n');
        if (line == "\r")
            break;
    }

    String jsonPayload = readHttpBodySkippingChunkMarkers(weatherClient);
    weatherClient.stop();

    DynamicJsonDocument doc(2048);
    DeserializationError error = deserializeJson(doc, jsonPayload);
    if (error)
    {
        Serial.print("Weather JSON parse error: ");
        Serial.println(error.c_str());
        return false;
    }

    temperatureOut = doc["current_weather"]["temperature"] | 0.0f;
    int weatherCode = doc["current_weather"]["weathercode"] | -1;
    timeStrOut = doc["current_weather"]["time"].as<String>();
    weatherDescOut = weatherCodeToString(weatherCode);

    return true;
}

String weatherCodeToString(int code)
{
    switch (code)
    {
    case 0: return "clear";
    case 1: return "mostly clear";
    case 2: return "partly cloudy";
    case 3: return "overcast";
    case 45:
    case 48: return "foggy";
    case 51:
    case 53:
    case 55: return "drizzling";
    case 61:
    case 63:
    case 65: return "raining";
    case 80:
    case 81:
    case 82: return "showery";
    default: return "weirdly unclassifiable";
    }
}

String getOpenAIResponse(const String &prompt)
{
    openaiClient.setInsecure();

    if (!openaiClient.connect("api.openai.com", 443))
    {
        Serial.println("Connection to OpenAI failed");
        return "";
    }

    DynamicJsonDocument reqDoc(2048);
    reqDoc["model"] = openai_model;

    JsonArray messages = reqDoc.createNestedArray("messages");
    JsonObject message = messages.createNestedObject();
    message["role"] = "user";
    message["content"] = prompt;

    reqDoc["max_tokens"] = 120;
    reqDoc["temperature"] = 0.9;

    String requestBody;
    serializeJson(reqDoc, requestBody);

    Serial.println("Sending request to OpenAI...");

    openaiClient.println("POST /v1/chat/completions HTTP/1.1");
    openaiClient.println("Host: api.openai.com");
    openaiClient.println("Authorization: Bearer " + String(openai_key));
    openaiClient.println("Content-Type: application/json");
    openaiClient.print("Content-Length: ");
    openaiClient.println(requestBody.length());
    openaiClient.println("Connection: close");
    openaiClient.println();
    openaiClient.println(requestBody);

    if (!waitForClientData(openaiClient, 30000))
    {
        Serial.println("No response from OpenAI - timeout");
        openaiClient.stop();
        return "";
    }

    while (openaiClient.connected())
    {
        String line = openaiClient.readStringUntil('\n');
        if (line == "\r")
            break;
    }

    String response = openaiClient.readString();
    openaiClient.stop();

    Serial.println("Raw response:");
    Serial.println(response);

    DynamicJsonDocument jsonDoc(8192);
    DeserializationError error = deserializeJson(jsonDoc, response);
    if (error)
    {
        Serial.print("OpenAI JSON parsing failed: ");
        Serial.println(error.c_str());
        return "";
    }

    if (!jsonDoc.containsKey("choices"))
    {
        Serial.println("Unexpected OpenAI response format");
        if (jsonDoc.containsKey("error"))
        {
            Serial.println(jsonDoc["error"]["message"].as<String>());
        }
        return "";
    }

    return jsonDoc["choices"][0]["message"]["content"].as<String>();
}

bool waitForClientData(WiFiClientSecure &client, uint32_t timeoutMs)
{
    uint32_t start = millis();
    while (!client.available() && (millis() - start < timeoutMs))
    {
        delay(100);
    }
    return client.available();
}

String readHttpBodySkippingChunkMarkers(WiFiClientSecure &client)
{
    String body;

    while (client.available())
    {
        String line = client.readStringUntil('\n');
        line.trim();

        if (line.length() == 0)
            continue;

        // Skip terminal chunk marker
        if (line == "0")
            continue;

        // Skip chunk-size headers like "1a3" or "7F"
        bool isHexOnly = true;
        for (size_t i = 0; i < line.length(); i++)
        {
            if (!isHexadecimalDigit(line[i]))
            {
                isHexOnly = false;
                break;
            }
        }

        if (isHexOnly)
            continue;

        body += line;
    }

    return body;
}