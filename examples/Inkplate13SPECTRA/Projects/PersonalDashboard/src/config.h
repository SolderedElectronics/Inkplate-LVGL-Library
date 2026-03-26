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
#define WIFI_SSID "Soldered Electronics"
#define WIFI_PASS "dasduino"

// --- Google Calendar ---
// Calendar ID: Google Calendar settings > "Integrate calendar" > Calendar ID
// API Key: console.cloud.google.com > APIs & Services > Credentials > Create API key
// Make sure Google Calendar API is enabled and your calendar is set to public
#define CALENDAR_ID  "0993f590ad60b390645331333129b9ae7cdfad3eabb2b143b5b716d848b1e241@group.calendar.google.com"
#define CALENDAR_KEY "AIzaSyA9Zi7rVHQl_yQScNzpn__UIMhc37SBsfc"

// --- Location (for OpenMeteo) ---
#define LATITUDE   45.5550f  // Latitude of your city
#define LONGITUDE  18.6955f  // Longitude of your city
#define TIMEZONE   2         // UTC offset (e.g. 2 for Osijek/CEST, -4 for NYC)

#endif
