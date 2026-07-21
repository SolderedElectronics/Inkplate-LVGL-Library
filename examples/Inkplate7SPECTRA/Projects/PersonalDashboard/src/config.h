#ifndef CONFIG_H
#define CONFIG_H

// --- User ---
#define USERNAME "My Personal Dashboard"

// --- Pokemon ---
// Pokedex number of the Pokemon to display (e.g. 149 = Dragonite).
// To change it:
//   1. Go to https://lvgl.io/tools/imageconverter (select LVGL v9, output: C array)
//      and convert the sprite image for your chosen Pokemon.
//   2. Replace src/149.c with the new file, rename the generated variable and
//      array to `pokemon_sprite` and `pokemon_sprite_map` respectively.
#define POKEMON_ID 149

// --- WiFi ---
#define WIFI_SSID "your-wifi-ssid"
#define WIFI_PASS "your-wifi-password"

// --- Google Calendar ---
// Calendar ID: Google Calendar settings > "Integrate calendar" > Calendar ID
// API Key: console.cloud.google.com > APIs & Services > Credentials > Create API key
// Make sure Google Calendar API is enabled and your calendar is set to public
#define CALENDAR_ID  "your-calendar-id@group.calendar.google.com"
#define CALENDAR_KEY "your-google-api-key"

// --- Location (for OpenMeteo) ---
#define LATITUDE   0.0000f   // Latitude of your city
#define LONGITUDE  0.0000f   // Longitude of your city
#define TIMEZONE   0         // UTC offset (e.g. 2 for CEST, -5 for EST, -8 for PST)

#endif
