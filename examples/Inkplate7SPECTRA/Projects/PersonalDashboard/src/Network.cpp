#include "Network.h"
#include <time.h>

bool DataFetcher::connectWiFi(const char *ssid, const char *pass)
{
    WiFi.begin(ssid, pass);
    unsigned long start = millis();
    while (WiFi.status() != WL_CONNECTED && millis() - start < 30000)
    {
        delay(500);
    }
    return WiFi.status() == WL_CONNECTED;
}

String DataFetcher::httpGet(const String &url)
{
    HTTPClient http;
    http.begin(url);
    http.setTimeout(10000);
    int code = http.GET();
    String body = "";
    if (code > 0)
        body = http.getString();
    else
        Serial.printf("[Network] GET failed: %s  code=%d\n", url.c_str(), code);
    http.end();
    return body;
}

bool DataFetcher::httpGetBinary(const String &url, uint8_t *&outData, size_t &outLen)
{
    HTTPClient http;
    http.begin(url);
    http.setTimeout(15000);
    int code = http.GET();
    if (code != 200)
    {
        Serial.printf("[Network] Binary GET failed: code=%d\n", code);
        http.end();
        return false;
    }
    WiFiClient *stream = http.getStreamPtr();
    outLen = http.getSize();
    if (outLen <= 0 || outLen > 100000) // sanity check: sprites are ~7KB
    {
        Serial.println("[Network] Binary size out of range");
        http.end();
        return false;
    }
    outData = (uint8_t *)ps_malloc(outLen);
    if (!outData)
    {
        Serial.println("[Network] ps_malloc failed for sprite");
        http.end();
        return false;
    }
    size_t received = 0;
    unsigned long t = millis();
    while (received < outLen && millis() - t < 10000)
    {
        if (stream->available())
        {
            outData[received++] = stream->read();
        }
    }
    http.end();
    if (received < outLen)
    {
        free(outData);
        outData = nullptr;
        return false;
    }
    return true;
}

bool DataFetcher::fetchWeather(WeatherInfo &out)
{
    char url[320];
    snprintf(url, sizeof(url),
             "https://api.open-meteo.com/v1/forecast"
             "?latitude=%.4f&longitude=%.4f"
             "&current=temperature_2m,weather_code,wind_speed_10m,precipitation"
             "&daily=sunrise,sunset"
             "&timezone=auto&forecast_days=1",
             (float)LATITUDE, (float)LONGITUDE);

    String body = httpGet(url);
    if (body.isEmpty())
        return false;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
    {
        Serial.println("[Network] Weather JSON parse failed");
        return false;
    }

    out.tempC        = doc["current"]["temperature_2m"].as<float>();
    out.weatherCode  = doc["current"]["weather_code"].as<int>();
    out.windSpeed    = doc["current"]["wind_speed_10m"].as<float>();
    out.precipitation = doc["current"]["precipitation"].as<float>();

    // sunrise/sunset come as "2025-01-12T06:45" — extract HH:MM (used by Day Cycle arc)
    String sr = doc["daily"]["sunrise"][0].as<String>();
    String ss = doc["daily"]["sunset"][0].as<String>();
    out.sunrise = sr.length() >= 16 ? sr.substring(11, 16) : sr;
    out.sunset  = ss.length() >= 16 ? ss.substring(11, 16) : ss;
    out.valid   = true;
    return true;
}

bool DataFetcher::fetchCalendar(CalendarEvent events[], int &count, int maxResults)
{
    count = 0;

    // Wait for NTP sync (matches reference library approach)
    struct tm timeinfo;
    int attempts = 0;
    while (!getLocalTime(&timeinfo) && attempts < 10)
    {
        delay(1000);
        attempts++;
    }
    if (!getLocalTime(&timeinfo))
    {
        Serial.println("[Network] Calendar: time not available");
        return false;
    }

    // timeMin = today at 00:00:00 — includes ongoing events that started earlier today
    char timeMin[32];
    strftime(timeMin, sizeof(timeMin), "%Y-%m-%dT00:00:00Z", &timeinfo);

    // timeMax = today + 14 days (2-week window)
    time_t now = mktime(&timeinfo);
    now += 14 * 24 * 60 * 60;
    struct tm timeMaxInfo;
    gmtime_r(&now, &timeMaxInfo);
    char timeMax[32];
    strftime(timeMax, sizeof(timeMax), "%Y-%m-%dT23:59:59Z", &timeMaxInfo);

    char url[600];
    snprintf(url, sizeof(url),
             "https://www.googleapis.com/calendar/v3/calendars/%s/events"
             "?key=%s&maxResults=%d&orderBy=startTime"
             "&singleEvents=true&timeMin=%s&timeMax=%s",
             CALENDAR_ID, CALENDAR_KEY, maxResults, timeMin, timeMax);

    String body = httpGet(url);
    if (body.isEmpty())
        return false;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
    {
        Serial.println("[Network] Calendar JSON parse failed");
        return false;
    }

    JsonArray items = doc["items"].as<JsonArray>();
    for (JsonObject item : items)
    {
        if (count >= maxResults)
            break;
        events[count].title = item["summary"].as<String>();

        // Timed event has "dateTime", all-day has "date"
        String dt = item["start"]["dateTime"].as<String>();
        String dateStr;
        if (dt.isEmpty())
        {
            dateStr = item["start"]["date"].as<String>(); // "YYYY-MM-DD"
            events[count].startTime = formatDateFromISO(dateStr);
            events[count].startISO  = dateStr;
            events[count].endTime   = item["end"]["date"].as<String>();
        }
        else
        {
            dateStr = dt.substring(0, 10); // extract "YYYY-MM-DD" from ISO timestamp
            events[count].startTime = formatTimeFromISO(dt);
            events[count].startISO  = dt;
            events[count].endTime   = item["end"]["dateTime"].as<String>();
        }
        events[count].dayLabel = dayLabelFromDate(dateStr);
        count++;
    }
    return true;
}

bool DataFetcher::fetchPokemon(PokemonInfo &out)
{
    int id = POKEMON_ID;

    char url[64];
    snprintf(url, sizeof(url), "https://pokeapi.co/api/v2/pokemon/%d", id);

    String body = httpGet(url);
    if (body.isEmpty())
        return false;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
    {
        Serial.println("[Network] Pokemon JSON parse failed");
        return false;
    }

    out.name    = doc["name"].as<String>();
    out.name[0] = toupper(out.name[0]);

    // Primary type
    out.type    = doc["types"][0]["type"]["name"].as<String>();
    out.type[0] = toupper(out.type[0]);

    // Secondary type (dual-type Pokemon)
    if (doc["types"].size() > 1)
    {
        out.type2    = doc["types"][1]["type"]["name"].as<String>();
        out.type2[0] = toupper(out.type2[0]);
    }

    out.height = doc["height"].as<int>(); // decimeters
    out.weight = doc["weight"].as<int>(); // hectograms

    // First non-hidden ability
    for (JsonObject ab : doc["abilities"].as<JsonArray>())
    {
        if (!ab["is_hidden"].as<bool>())
        {
            out.ability = ab["ability"]["name"].as<String>();
            out.ability.replace("-", " ");
            if (!out.ability.isEmpty()) out.ability[0] = toupper(out.ability[0]);
            break;
        }
    }

    // All key stats in one pass
    out.hp = out.attack = out.defense = out.speed = 0;
    for (JsonObject stat : doc["stats"].as<JsonArray>())
    {
        String sn = stat["stat"]["name"].as<String>();
        int    sv = stat["base_stat"].as<int>();
        if      (sn == "hp")      out.hp      = sv;
        else if (sn == "attack")  out.attack  = sv;
        else if (sn == "defense") out.defense = sv;
        else if (sn == "speed")   out.speed   = sv;
    }

    // Sprite is embedded as a static LVGL image (see src/149.c + pokemon_sprite.h)
    out.spriteData = nullptr;
    out.spriteLen  = 0;

    // Free the large Pokemon JSON before opening new HTTPS connections —
    // the full PokeAPI response includes moves, sprites etc. and can occupy
    // tens of KB of heap, leaving too little memory for the SSL session.
    doc.clear();
    body = "";  // also release the raw JSON string

    // Fetch weaknesses from PokeAPI type endpoint(s)
    // Collects unique types that deal 2× damage; fetches both types for dual-type Pokemon
    out.weaknessCount = 0;
    auto addWeaknesses = [&](const String &typeName) {
        String lt = typeName;
        lt.toLowerCase();
        String tb = httpGet("https://pokeapi.co/api/v2/type/" + lt);
        if (tb.isEmpty()) return;
        JsonDocument td;
        if (deserializeJson(td, tb) != DeserializationError::Ok) return;
        for (JsonObject t : td["damage_relations"]["double_damage_from"].as<JsonArray>())
        {
            if (out.weaknessCount >= 10) break;
            String wn = t["name"].as<String>();
            if (wn.isEmpty()) continue;
            wn[0] = toupper(wn[0]);
            bool dup = false;
            for (int j = 0; j < out.weaknessCount; j++)
                if (out.weaknesses[j] == wn) { dup = true; break; }
            if (!dup) out.weaknesses[out.weaknessCount++] = wn;
        }
    };
    addWeaknesses(out.type);
    if (!out.type2.isEmpty()) addWeaknesses(out.type2);

    // Fetch Pokedex flavor text from the species endpoint (small response, safe after types)
    {
        char speciesUrl[64];
        snprintf(speciesUrl, sizeof(speciesUrl),
                 "https://pokeapi.co/api/v2/pokemon-species/%d", id);
        String sb = httpGet(speciesUrl);
        if (!sb.isEmpty())
        {
            JsonDocument sd;
            if (deserializeJson(sd, sb) == DeserializationError::Ok)
            {
                for (JsonObject entry : sd["flavor_text_entries"].as<JsonArray>())
                {
                    if (entry["language"]["name"].as<String>() == "en")
                    {
                        out.description = entry["flavor_text"].as<String>();
                        // Game text uses \f (form feed) and \n as line breaks — flatten to spaces
                        out.description.replace("\f", " ");
                        out.description.replace("\n", " ");
                        out.description.replace("\r", " ");
                        while (out.description.indexOf("  ") >= 0)
                            out.description.replace("  ", " ");
                        out.description.trim();
                        break;
                    }
                }
            }
        }
    }

    out.valid = true;
    return true;
}

String DataFetcher::formatTimeFromISO(const String &iso)
{
    // "2025-01-12T14:30:00+02:00" → "14:30"
    int tPos = iso.indexOf('T');
    if (tPos < 0 || iso.length() < (size_t)(tPos + 6))
        return iso;
    return iso.substring(tPos + 1, tPos + 6);
}

String DataFetcher::formatDateFromISO(const String &iso)
{
    // "2025-01-12" → "12.01."
    if (iso.length() < 10)
        return iso;
    String day   = iso.substring(8, 10);
    String month = iso.substring(5, 7);
    return day + "." + month + ".";
}

String DataFetcher::dayLabelFromDate(const String &dateStr)
{
    // "YYYY-MM-DD" → "Today" | "Tomorrow" | "Mon"…"Sun" | "DD.MM."
    if (dateStr.length() < 10)
        return dateStr;

    struct tm evTm = {};
    evTm.tm_year = dateStr.substring(0, 4).toInt() - 1900;
    evTm.tm_mon  = dateStr.substring(5, 7).toInt() - 1;
    evTm.tm_mday = dateStr.substring(8, 10).toInt();
    time_t evTime = mktime(&evTm); // also fills tm_wday

    time_t nowTime = time(nullptr);
    struct tm nowTm = *localtime(&nowTime);
    nowTm.tm_hour = 0;
    nowTm.tm_min  = 0;
    nowTm.tm_sec  = 0;
    int diff = (int)((evTime - mktime(&nowTm)) / 86400);

    if (diff == 0) return "Today";
    if (diff == 1) return "Tomorrow";
    if (diff > 1 && diff < 7)
    {
        const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
        return days[evTm.tm_wday];
    }
    char buf[8];
    snprintf(buf, sizeof(buf), "%02d.%02d.", evTm.tm_mday, evTm.tm_mon + 1);
    return buf;
}

bool DataFetcher::fetchQuote(StoicQuote &out)
{
    String body = httpGet("https://stoic.tekloon.net/stoic-quote");
    if (body.isEmpty())
        return false;

    JsonDocument doc;
    if (deserializeJson(doc, body) != DeserializationError::Ok)
    {
        Serial.println("[Network] Quote JSON parse failed");
        return false;
    }

    out.text   = doc["data"]["quote"].as<String>();
    out.author = doc["data"]["author"].as<String>();
    out.valid  = (!out.text.isEmpty() && !out.author.isEmpty());
    return out.valid;
}
