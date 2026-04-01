/*
    Network.cpp
    Inkplate 4TEMPERA Arduino library
    Matej Andračić @ Soldered
    May 5, 2025
*/

#include "Network.h"
#include "Inkplate-LVGL.h"
#include <ArduinoJson.h>

StaticJsonDocument<35000> doc;

void NetworkFunctions::setCredentials(const char *wifiSSID, const char *wifiPass, const char *apiKey)
{
    strncpy(ssid, wifiSSID, sizeof(ssid) - 1);
    ssid[sizeof(ssid) - 1] = '\0';

    strncpy(pass, wifiPass, sizeof(pass) - 1);
    pass[sizeof(pass) - 1] = '\0';

    strncpy(api_key_news, apiKey, sizeof(api_key_news) - 1);
    api_key_news[sizeof(api_key_news) - 1] = '\0';
}

void NetworkFunctions::setTimeZone(int tz)
{
    timeZone = tz;
}

void NetworkFunctions::begin()
{
    WiFi.begin(ssid, pass);
    Serial.println(F("Connecting to WiFi..."));

    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.print(F("."));
    }

    Serial.println(F("\nWiFi connected."));
}

struct news *NetworkFunctions::getData()
{
    struct news *ent = nullptr;

    if (WiFi.status() != WL_CONNECTED)
    {
        WiFi.reconnect();
        delay(5000);

        int cnt = 0;
        Serial.println(F("Waiting for WiFi to reconnect..."));
        while (WiFi.status() != WL_CONNECTED)
        {
            Serial.print(F("."));
            delay(1000);
            ++cnt;

            if (cnt == 7)
            {
                Serial.println(F("Can't connect to WiFi, restarting..."));
                delay(100);
                ESP.restart();
            }
        }
    }

    bool sleep = WiFi.getSleep();
    WiFi.setSleep(false);

    HTTPClient http;
    http.getStream().setTimeout(10);
    http.getStream().flush();

    char url[192];
    snprintf(url, sizeof(url),
             "https://newsapi.org/v2/top-headlines?country=us&apiKey=%s",
             api_key_news);

    http.begin(url);

    int httpCode = http.GET();
    Serial.printf("HTTP code: %d\n", httpCode);

    if (httpCode == 200)
    {
        while (http.getStream().available() && http.getStream().peek() != '{')
        {
            (void)http.getStream().read();
        }

        DeserializationError error = deserializeJson(doc, http.getStream());
        if (error)
        {
            Serial.print(F("deserializeJson() failed: "));
            Serial.println(error.c_str());
            http.end();
            WiFi.setSleep(sleep);
            return nullptr;
        }

        const char *status = doc["status"];
        if (!status || strcmp(status, "ok") != 0)
        {
            Serial.println(F("News API returned invalid status."));
            http.end();
            WiFi.setSleep(sleep);
            doc.clear();
            return nullptr;
        }

        JsonArray articles = doc["articles"].as<JsonArray>();
        int n = articles.size();

        Serial.printf("Number of articles: %d\n", n);

        if (n <= 0)
        {
            http.end();
            WiFi.setSleep(sleep);
            doc.clear();
            return nullptr;
        }

        ent = (struct news *)ps_malloc((n + 1) * sizeof(struct news));
        if (!ent)
        {
            Serial.println(F("Memory allocation failed for news array!"));
            http.end();
            WiFi.setSleep(sleep);
            doc.clear();
            return nullptr;
        }

        memset(ent, 0, (n + 1) * sizeof(struct news));

        int i = 0;
        for (JsonObject article : articles)
        {
            const char *temp_title = article["title"];
            const char *temp_description = article["description"];

            if (temp_title)
            {
                size_t title_len = strlen(temp_title) + 1;
                ent[i].title = (char *)malloc(title_len);
                if (ent[i].title)
                {
                    memcpy(ent[i].title, temp_title, title_len);
                }
            }

            if (temp_description)
            {
                size_t description_len = strlen(temp_description) + 1;
                ent[i].description = (char *)malloc(description_len);
                if (ent[i].description)
                {
                    memcpy(ent[i].description, temp_description, description_len);
                }
            }

            i++;
        }

        // Null-terminate the array for safer iteration
        ent[n].title = nullptr;
        ent[n].description = nullptr;
    }
    else
    {
        Serial.printf("HTTP request failed, code: %d\n", httpCode);
    }

    doc.clear();
    http.end();
    WiFi.setSleep(sleep);

    return ent;
}