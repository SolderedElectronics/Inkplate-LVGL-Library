#include "Gui.h"
#include "pokemon_sprite.h"
#include <time.h>

// ── Bento grid constants (all in px) ─────────────────────────────────────────
#define GAP    25   // gap between every card and screen edge

// Screen
#define SW 1600
#define SH 1200

// Header row (username, no card — just text on black bg)
#define HDR_Y  GAP
#define HDR_H  55

// Content area starts below header
#define BODY_Y (HDR_Y + HDR_H + GAP)   // 105
#define BODY_H (SH - BODY_Y - GAP)     // 1070

// Column widths
#define COL_L  550
#define COL_M  600
#define COL_R  (SW - 2*GAP - COL_L - COL_M - 2*GAP)  // 500

// Column x positions
#define LEFT_X  GAP                        // 25
#define MID_X   (LEFT_X + COL_L + GAP)    // 450
#define RIGHT_X (MID_X  + COL_M + GAP)    // 1075

// Row heights (left + right columns are split)
#define ROW_T  490
#define ROW_B  (BODY_H - ROW_T - GAP)     // 555

// Row y positions
#define TOP_Y  BODY_Y                      // 105
#define BOT_Y  (TOP_Y + ROW_T + GAP)      // 620

// Middle column: calendar + quote (stacked)
#define QUOTE_H  200
#define CAL_H    (BODY_H - GAP - QUOTE_H)  // 845
#define QUOTE_Y  (TOP_Y + CAL_H + GAP)     // 975

// Card corner radius
#define RADIUS 24

// ── Colors ───────────────────────────────────────────────────────────────────
static const lv_color_t C_BG     = lv_color_hex(0x6390F0); // water-blue background
static const lv_color_t C_CARD   = lv_color_hex(0xFFFFFF); // all cards
static const lv_color_t C_BLACK  = lv_color_hex(0x000000);
static const lv_color_t C_WHITE  = lv_color_hex(0xFFFFFF);
static const lv_color_t C_GRAY   = lv_color_hex(0x888888);
static const lv_color_t C_LGRAY  = lv_color_hex(0xE0E0E0);
static const lv_color_t C_YELLOW = lv_color_hex(0xFFCC00);
static const lv_color_t C_BLUE   = lv_color_hex(0x0055CC);
static const lv_color_t C_GREEN  = lv_color_hex(0x00AA44);
static const lv_color_t C_RED    = lv_color_hex(0xCC2200);
#define PAD 20  // inner padding inside cards

// ── Helpers ───────────────────────────────────────────────────────────────────

lv_obj_t *Gui::createCard(lv_obj_t *parent, int x, int y, int w, int h,
                           lv_color_t bg, int radius)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, bg, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, radius, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    lv_obj_set_scrollbar_mode(card, LV_SCROLLBAR_MODE_OFF);
    return card;
}

lv_obj_t *Gui::createLabel(lv_obj_t *parent, const char *text,
                            const lv_font_t *font, lv_color_t color,
                            int x, int y)
{
    lv_obj_t *lbl = lv_label_create(parent);
    lv_label_set_text(lbl, text);
    lv_obj_set_style_text_font(lbl, font, 0);
    lv_obj_set_style_text_color(lbl, color, 0);
    lv_obj_set_pos(lbl, x, y);
    return lbl;
}

// ── Text sanitiser — replaces non-ASCII (curly quotes, en/em dashes, ellipsis)
//    with plain ASCII so the Montserrat bitmap font renders them correctly. ────

static String sanitizeText(const String &s)
{
    String out;
    out.reserve(s.length());
    const uint8_t *p = (const uint8_t *)s.c_str();
    int len = (int)s.length();
    for (int i = 0; i < len; )
    {
        uint8_t c = p[i];
        // UTF-8 E2 80 XX — covers most common typographic punctuation
        if (c == 0xE2 && i + 2 < len && p[i + 1] == 0x80)
        {
            uint8_t c2 = p[i + 2];
            if      (c2 == 0x98 || c2 == 0x99) { out += '\'';  } // ' '
            else if (c2 == 0x9C || c2 == 0x9D) { out += '"';   } // " "
            else if (c2 == 0x93 || c2 == 0x94) { out += '-';   } // – —
            else if (c2 == 0xA6)               { out += "..."; } // …
            i += 3;
        }
        else if (c >= 0x80)
        {
            // Skip any other multi-byte sequence silently
            if      ((c & 0xE0) == 0xC0) i += 2;
            else if ((c & 0xF0) == 0xE0) i += 3;
            else if ((c & 0xF8) == 0xF0) i += 4;
            else                         i += 1;
        }
        else
        {
            out += (char)c;
            i++;
        }
    }
    return out;
}

// ── init ──────────────────────────────────────────────────────────────────────

void Gui::init()
{
    // Black background
    lv_obj_set_style_bg_color(lv_screen_active(), C_BG, LV_PART_MAIN);

    // Username label (top-left, on black)
    createLabel(lv_screen_active(), USERNAME,
                &lv_font_montserrat_36, C_WHITE, LEFT_X, HDR_Y + 10);

    // ── Five bento cards ──────────────────────────────────────────────────────

    // Pokemon sprite  (left-top)
    _pokemonImgPanel = createCard(lv_screen_active(),
                                  LEFT_X, TOP_Y, COL_L, ROW_T, C_CARD, RADIUS);

    // Pokemon info    (left-bottom)
    _pokemonPanel    = createCard(lv_screen_active(),
                                  LEFT_X, BOT_Y, COL_L, ROW_B, C_CARD, RADIUS);

    // Calendar        (middle, top portion)
    _calendarPanel   = createCard(lv_screen_active(),
                                  MID_X, TOP_Y, COL_M, CAL_H, C_CARD, RADIUS);

    // Daily Quote     (middle, bottom portion)
    _quotePanel      = createCard(lv_screen_active(),
                                  MID_X, QUOTE_Y, COL_M, QUOTE_H, C_CARD, RADIUS);

    // Weather         (right-top)
    _weatherPanel    = createCard(lv_screen_active(),
                                  RIGHT_X, TOP_Y, COL_R, ROW_T, C_CARD, RADIUS);

    // Day cycle       (right-bottom)
    _dayCyclePanel   = createCard(lv_screen_active(),
                                  RIGHT_X, BOT_Y, COL_R, ROW_B, C_CARD, RADIUS);

    // Status label shown while loading (centred on screen)
    _statusLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(_statusLabel, "Connecting...");
    lv_obj_set_style_text_font(_statusLabel, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(_statusLabel, C_WHITE, 0);
    lv_obj_align(_statusLabel, LV_ALIGN_CENTER, 0, 0);
}

void Gui::showConnecting()
{
    if (_statusLabel)
        lv_label_set_text(_statusLabel, "Connecting to WiFi...");
}

void Gui::showWifiError()
{
    if (_statusLabel)
        lv_label_set_text(_statusLabel, "WiFi connection failed!");
}

// ── render ────────────────────────────────────────────────────────────────────

void Gui::render(WeatherInfo &weather, CalendarEvent events[], int eventCount,
                 PokemonInfo &pokemon, StoicQuote &quote)
{
    if (_statusLabel)
        lv_obj_add_flag(_statusLabel, LV_OBJ_FLAG_HIDDEN);

    renderPokemonSprite(pokemon);
    renderPokemonInfo(pokemon);
    renderCalendar(events, eventCount);
    renderWeather(weather);
    renderDayCycle(weather);
    renderQuote(quote);
}

// ── Pokemon sprite card ───────────────────────────────────────────────────────

void Gui::renderPokemonSprite(PokemonInfo &p)
{
    lv_obj_clean(_pokemonImgPanel);

    if (!p.valid)
    {
        // Placeholder
        lv_obj_t *box = createCard(_pokemonImgPanel,
                                   (COL_L - 160) / 2, (ROW_T - 160) / 2,
                                   160, 160, C_LGRAY, RADIUS);
        createLabel(box, "?", &lv_font_montserrat_48, C_GRAY, 60, 52);
        return;
    }

    // Use the statically embedded LVGL image (see src/149.c / pokemon_sprite.h)
    lv_obj_t *img = lv_image_create(_pokemonImgPanel);
    lv_image_set_src(img, &pokemon_sprite);
    lv_obj_align(img, LV_ALIGN_CENTER, 0, 0);
}

// ── Pokemon info card ─────────────────────────────────────────────────────────

void Gui::renderPokemonInfo(PokemonInfo &p)
{
    lv_obj_clean(_pokemonPanel);

    if (!p.valid)
    {
        createLabel(_pokemonPanel, "No Pokemon", &lv_font_montserrat_26, C_GRAY, PAD, PAD);
        return;
    }

    int y = PAD;

    // Name
    createLabel(_pokemonPanel, p.name.c_str(), &lv_font_montserrat_40, C_BLACK, PAD, y);
    y += 52;

    // Type badge(s) — proper type colors
    int t1w = p.type.length() * 13 + 26;
    lv_obj_t *b1 = createCard(_pokemonPanel, PAD, y, t1w, 34, typeColor(p.type), 17);
    createLabel(b1, p.type.c_str(), &lv_font_montserrat_20, C_WHITE, 13, 7);
    if (!p.type2.isEmpty())
    {
        int t2w = p.type2.length() * 13 + 26;
        lv_obj_t *b2 = createCard(_pokemonPanel, PAD + t1w + 8, y, t2w, 34,
                                  typeColor(p.type2), 17);
        createLabel(b2, p.type2.c_str(), &lv_font_montserrat_20, C_WHITE, 13, 7);
    }
    y += 46;

    // Divider
    lv_obj_t *div = createCard(_pokemonPanel, PAD, y, COL_L - 2*PAD, 2, C_LGRAY, 0);
    (void)div;
    y += 14;

    // Stats grid — two columns
    char buf[40];
    snprintf(buf, sizeof(buf), "HP   %3d", p.hp);
    createLabel(_pokemonPanel, buf, &lv_font_montserrat_26, C_BLACK, PAD, y);
    snprintf(buf, sizeof(buf), "Atk  %3d", p.attack);
    createLabel(_pokemonPanel, buf, &lv_font_montserrat_26, C_BLACK, COL_L / 2, y);
    y += 36;

    snprintf(buf, sizeof(buf), "Def  %3d", p.defense);
    createLabel(_pokemonPanel, buf, &lv_font_montserrat_26, C_BLACK, PAD, y);
    snprintf(buf, sizeof(buf), "Spd  %3d", p.speed);
    createLabel(_pokemonPanel, buf, &lv_font_montserrat_26, C_BLACK, COL_L / 2, y);
    y += 42;

    // Height & weight
    snprintf(buf, sizeof(buf), "%.1f m  /  %.1f kg", p.height / 10.0f, p.weight / 10.0f);
    createLabel(_pokemonPanel, buf, &lv_font_montserrat_24, C_GRAY, PAD, y);
    y += 36;

    // Ability
    if (!p.ability.isEmpty())
    {
        String abilStr = "Ability: " + p.ability;
        createLabel(_pokemonPanel, abilStr.c_str(), &lv_font_montserrat_24, C_BLACK, PAD, y);
        y += 36;
    }

    if (p.weaknessCount > 0)
    {
        // Divider
        lv_obj_t *div2 = createCard(_pokemonPanel, PAD, y, COL_L - 2*PAD, 2, C_LGRAY, 0);
        (void)div2;
        y += 14;

        createLabel(_pokemonPanel, "Weak to:", &lv_font_montserrat_24, C_BLACK, PAD, y);
        y += 32;

        // Weakness badges — flow layout, wrap to next row if needed
        int bx = PAD;
        for (int j = 0; j < p.weaknessCount; j++)
        {
            int bw = p.weaknesses[j].length() * 12 + 20;
            if (bx + bw > COL_L - PAD)
            {
                bx = PAD;
                y += 36;
            }
            lv_obj_t *wb = createCard(_pokemonPanel, bx, y, bw, 28,
                                      typeColor(p.weaknesses[j]), 14);
            createLabel(wb, p.weaknesses[j].c_str(), &lv_font_montserrat_18, C_WHITE, 10, 4);
            bx += bw + 6;
        }
    }
}

// ── Calendar card ─────────────────────────────────────────────────────────────

void Gui::renderCalendar(CalendarEvent events[], int count)
{
    lv_obj_clean(_calendarPanel);

    // Title
    createLabel(_calendarPanel, "Calendar", &lv_font_montserrat_32, C_BLACK, PAD, PAD);

    // Divider
    lv_obj_t *div = createCard(_calendarPanel, PAD, 54, COL_M - 2*PAD, 2, C_LGRAY, 0);
    (void)div;

    if (count == 0)
    {
        createLabel(_calendarPanel, "No upcoming events",
                    &lv_font_montserrat_24, C_GRAY, PAD, 76);
        return;
    }

    // C_GREEN is reserved for "ongoing" marker — not in cycling set
    static const lv_color_t dotColors[] = {C_BLUE, C_RED, C_YELLOW, C_GRAY, C_BLUE};

    int y = 68;
    String lastDayLabel = "";

    for (int i = 0; i < count; i++)
    {
        // Day group header — colored badge, rendered once per unique day
        if (events[i].dayLabel != lastDayLabel)
        {
            if (i > 0) y += 10;
            int dw = (int)(events[i].dayLabel.length() * 14 * 1.3f) + 24;
            lv_obj_t *dayBadge = createCard(_calendarPanel, PAD, y, dw, 36,
                                            dayColor(events[i].dayLabel), 18);
            createLabel(dayBadge, events[i].dayLabel.c_str(),
                        &lv_font_montserrat_24, C_WHITE, 12, 6);
            y += 46;
            lastDayLabel = events[i].dayLabel;
        }

        // Dot — green if currently ongoing, otherwise cycling color
        bool ongoing = isCurrentEvent(events[i].startISO, events[i].endTime);
        lv_obj_t *dot = createCard(_calendarPanel, PAD, y + 4, 12, 12,
                                   ongoing ? C_GREEN : dotColors[i % 5], LV_RADIUS_CIRCLE);
        (void)dot;

        // Start time (black)
        createLabel(_calendarPanel, events[i].startTime.c_str(),
                    &lv_font_montserrat_20, C_BLACK, PAD + 20, y);

        // End time (gray) — extracted from raw ISO if it has a time part
        int tPos = events[i].endTime.indexOf('T');
        if (tPos >= 0 && (int)events[i].endTime.length() >= tPos + 6)
        {
            String endFmt = " - " + events[i].endTime.substring(tPos + 1, tPos + 6);
            createLabel(_calendarPanel, endFmt.c_str(),
                        &lv_font_montserrat_20, C_GRAY, PAD + 20 + 58, y);
        }

        // Title
        String title = sanitizeText(events[i].title);
        if (title.length() > 34) title = title.substring(0, 33) + "...";
        createLabel(_calendarPanel, title.c_str(),
                    &lv_font_montserrat_24, C_BLACK, PAD + 20, y + 24);

        // Separator
        lv_obj_t *sep = createCard(_calendarPanel, PAD, y + 58,
                                   COL_M - 2*PAD, 1, C_LGRAY, 0);
        (void)sep;
        y += 66;
    }
}

// ── Weather card ──────────────────────────────────────────────────────────────

void Gui::renderWeather(WeatherInfo &w)
{
    lv_obj_clean(_weatherPanel);

    createLabel(_weatherPanel, "Weather", &lv_font_montserrat_32, C_BLACK, PAD, PAD);

    lv_obj_t *div = createCard(_weatherPanel, PAD, 54, COL_R - 2*PAD, 2, C_LGRAY, 0);
    (void)div;

    if (!w.valid)
    {
        createLabel(_weatherPanel, "No data", &lv_font_montserrat_26, C_GRAY, PAD, 76);
        return;
    }

    // Temperature (large)
    char buf[24];
    snprintf(buf, sizeof(buf), "%.1f °C", w.tempC);
    createLabel(_weatherPanel, buf, &lv_font_montserrat_48, C_BLACK, PAD, 68);

    // Description
    const char *desc = wmoDescription(w.weatherCode);
    createLabel(_weatherPanel, desc, &lv_font_montserrat_26, wmoColor(w.weatherCode),
                PAD, 136);

    // Wind speed
    char buf2[40];
    snprintf(buf2, sizeof(buf2), "Wind: %.1f km/h", w.windSpeed);
    createLabel(_weatherPanel, buf2, &lv_font_montserrat_24, C_BLACK, PAD, 200);

    // Precipitation
    if (w.precipitation > 0.0f)
        snprintf(buf2, sizeof(buf2), "Precip: %.1f mm", w.precipitation);
    else
        strcpy(buf2, "Precip: none");
    createLabel(_weatherPanel, buf2, &lv_font_montserrat_24, C_BLACK, PAD, 238);
}

// ── Day cycle card ────────────────────────────────────────────────────────────

void Gui::renderDayCycle(WeatherInfo &w)
{
    lv_obj_clean(_dayCyclePanel);

    createLabel(_dayCyclePanel, "Day Cycle", &lv_font_montserrat_32, C_BLACK, PAD, PAD);

    lv_obj_t *div = createCard(_dayCyclePanel, PAD, 54, COL_R - 2*PAD, 2, C_LGRAY, 0);
    (void)div;

    if (!w.valid)
    {
        createLabel(_dayCyclePanel, "No data", &lv_font_montserrat_26, C_GRAY, PAD, 76);
        return;
    }

    // Labels pinned to bottom of card
    static const int LBL_H   = 26;  // approx height of font_22
    static const int LBL_GAP = 8;   // gap between sunrise and sunset rows
    int ssY    = ROW_B - PAD - LBL_H;              // sunset label y
    int srY    = ssY - LBL_GAP - LBL_H;            // sunrise label y

    // Arc centred in the space between the divider and the labels
    int arcTop  = 56 + 10;                          // 10px below divider
    int arcBot  = srY - 10;                         // 10px above sunrise label
    int arcSize = min(COL_R - 2*PAD - 10, arcBot - arcTop);
    int arcX    = (COL_R - arcSize) / 2;
    int arcY    = arcTop + (arcBot - arcTop - arcSize) / 2;

    // Night arc (blue, full circle — drawn first as background)
    lv_obj_t *bgArc = lv_arc_create(_dayCyclePanel);
    lv_obj_set_size(bgArc, arcSize, arcSize);
    lv_obj_set_pos(bgArc, arcX, arcY);
    lv_arc_set_bg_angles(bgArc, 0, 360);
    lv_arc_set_range(bgArc, 0, 100);
    lv_arc_set_value(bgArc, 100);
    lv_obj_set_style_arc_color(bgArc, C_BLUE, LV_PART_INDICATOR);
    lv_obj_set_style_arc_color(bgArc, C_BLUE, LV_PART_MAIN);
    lv_obj_set_style_arc_width(bgArc, 32, LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(bgArc, 32, LV_PART_MAIN);
    lv_obj_remove_style(bgArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(bgArc, LV_OBJ_FLAG_CLICKABLE);

    int srMin = timeToMinutes(w.sunrise);
    int ssMin = timeToMinutes(w.sunset);
    int srDeg = (int)((long)srMin * 360 / 1440);
    int ssDeg = (int)((long)ssMin * 360 / 1440);

    // Day arc (yellow)
    lv_obj_t *dayArc = lv_arc_create(_dayCyclePanel);
    lv_obj_set_size(dayArc, arcSize, arcSize);
    lv_obj_set_pos(dayArc, arcX, arcY);
    lv_arc_set_bg_angles(dayArc, 0, 360);
    lv_arc_set_angles(dayArc, srDeg, ssDeg);
    lv_obj_set_style_arc_color(dayArc, C_YELLOW, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(dayArc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(dayArc, 32, LV_PART_INDICATOR);
    lv_obj_remove_style(dayArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(dayArc, LV_OBJ_FLAG_CLICKABLE);

    // Now marker (red)
    time_t now     = time(nullptr);
    struct tm *tm  = localtime(&now);
    int nowMin     = tm->tm_hour * 60 + tm->tm_min;
    int nowDeg     = (int)((long)nowMin * 360 / 1440);

    lv_obj_t *nowArc = lv_arc_create(_dayCyclePanel);
    lv_obj_set_size(nowArc, arcSize, arcSize);
    lv_obj_set_pos(nowArc, arcX, arcY);
    lv_arc_set_bg_angles(nowArc, 0, 360);
    lv_arc_set_angles(nowArc, nowDeg, nowDeg + 5);
    lv_obj_set_style_arc_color(nowArc, C_RED, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(nowArc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_width(nowArc, 32, LV_PART_INDICATOR);
    lv_obj_remove_style(nowArc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(nowArc, LV_OBJ_FLAG_CLICKABLE);

    // Current time in centre of arc — "Current time" label + value below it
    int cx = arcX + arcSize / 2;
    int cy = arcY + arcSize / 2;
    createLabel(_dayCyclePanel, "Current time", &lv_font_montserrat_20, C_GRAY,
                cx - 60, cy - 42);
    char nowBuf[8];
    snprintf(nowBuf, sizeof(nowBuf), "%02d:%02d", tm->tm_hour, tm->tm_min);
    lv_obj_t *nowLbl = lv_label_create(_dayCyclePanel);
    lv_label_set_text(nowLbl, nowBuf);
    lv_obj_set_style_text_font(nowLbl, &lv_font_montserrat_28, 0);
    lv_obj_set_style_text_color(nowLbl, C_BLACK, 0);
    lv_obj_set_pos(nowLbl, cx - 35, cy - 18);

    // Sunrise / sunset labels pinned to bottom
    String srLbl = "Sunrise: " + w.sunrise;
    String ssLbl = "Sunset:  " + w.sunset;
    createLabel(_dayCyclePanel, srLbl.c_str(), &lv_font_montserrat_22, C_BLACK, PAD, srY);
    createLabel(_dayCyclePanel, ssLbl.c_str(), &lv_font_montserrat_22, C_BLACK, PAD, ssY);
}

// ── Daily Quote card ──────────────────────────────────────────────────────────

void Gui::renderQuote(StoicQuote &q)
{
    lv_obj_clean(_quotePanel);

    createLabel(_quotePanel, "Daily Quote", &lv_font_montserrat_32, C_BLACK, PAD, PAD);

    lv_obj_t *div = createCard(_quotePanel, PAD, 54, COL_M - 2*PAD, 2, C_LGRAY, 0);
    (void)div;

    if (!q.valid)
    {
        createLabel(_quotePanel, "No quote available", &lv_font_montserrat_24, C_GRAY, PAD, 70);
        return;
    }

    // Quote text — word-wrapped (sanitised to plain ASCII for font compatibility)
    lv_obj_t *qlbl = lv_label_create(_quotePanel);
    String cleanText = sanitizeText(q.text);
    lv_label_set_text(qlbl, cleanText.c_str());
    lv_obj_set_style_text_font(qlbl, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(qlbl, C_BLACK, 0);
    lv_obj_set_width(qlbl, COL_M - 2 * PAD);
    lv_label_set_long_mode(qlbl, LV_LABEL_LONG_WRAP);
    lv_obj_set_pos(qlbl, PAD, 70);

    // Author pinned to bottom
    String auth = "- " + sanitizeText(q.author);
    createLabel(_quotePanel, auth.c_str(), &lv_font_montserrat_20, C_GRAY,
                PAD, QUOTE_H - PAD - 24);
}

// ── WMO helpers ───────────────────────────────────────────────────────────────

const char *Gui::wmoDescription(int code)
{
    if (code == 0)   return "Clear sky";
    if (code <= 2)   return "Partly cloudy";
    if (code == 3)   return "Overcast";
    if (code <= 49)  return "Fog";
    if (code <= 57)  return "Drizzle";
    if (code <= 67)  return "Rain";
    if (code <= 77)  return "Snow";
    if (code <= 82)  return "Rain showers";
    if (code <= 86)  return "Snow showers";
    if (code == 95)  return "Thunderstorm";
    if (code <= 99)  return "Storm + hail";
    return "Unknown";
}

lv_color_t Gui::wmoColor(int code)
{
    if (code == 0)   return C_YELLOW;
    if (code <= 2)   return lv_color_hex(0xFFAA00);
    if (code <= 3)   return C_GRAY;
    if (code <= 49)  return lv_color_hex(0x999999);
    if (code <= 67)  return C_BLUE;
    if (code <= 77)  return lv_color_hex(0x99CCFF);
    if (code <= 86)  return lv_color_hex(0x66BBFF);
    return C_RED;
}

lv_color_t Gui::typeColor(const String &type)
{
    String t = type;
    t.toLowerCase();
    if (t == "fire")     return lv_color_hex(0xEE8130);
    if (t == "water")    return lv_color_hex(0x6390F0);
    if (t == "grass")    return lv_color_hex(0x7AC74C);
    if (t == "electric") return lv_color_hex(0xF7D02C);
    if (t == "ice")      return lv_color_hex(0x96D9D6);
    if (t == "fighting") return lv_color_hex(0xC22E28);
    if (t == "poison")   return lv_color_hex(0xA33EA1);
    if (t == "ground")   return lv_color_hex(0xE2BF65);
    if (t == "flying")   return lv_color_hex(0xA98FF3);
    if (t == "psychic")  return lv_color_hex(0xF95587);
    if (t == "bug")      return lv_color_hex(0xA6B91A);
    if (t == "rock")     return lv_color_hex(0xB6A136);
    if (t == "ghost")    return lv_color_hex(0x735797);
    if (t == "dragon")   return lv_color_hex(0x6F35FC);
    if (t == "dark")     return lv_color_hex(0x705746);
    if (t == "steel")    return lv_color_hex(0xB7B7CE);
    if (t == "fairy")    return lv_color_hex(0xD685AD);
    if (t == "normal")   return lv_color_hex(0xA8A878);
    return lv_color_hex(0x68A090);
}

lv_color_t Gui::dayColor(const String &label)
{
    if (label == "Today")     return lv_color_hex(0x1B5E20); // dark green
    if (label == "Tomorrow")  return lv_color_hex(0x0D47A1); // dark blue
    if (label == "Monday")    return lv_color_hex(0x4A148C); // dark purple
    if (label == "Tuesday")   return lv_color_hex(0x006064); // dark teal
    if (label == "Wednesday") return lv_color_hex(0xE65100); // dark orange
    if (label == "Thursday")  return lv_color_hex(0x4E342E); // dark brown
    if (label == "Friday")    return lv_color_hex(0xB71C1C); // dark red
    if (label == "Saturday")  return lv_color_hex(0x37474F); // dark blue-gray
    if (label == "Sunday")    return lv_color_hex(0x880E4F); // dark rose
    return lv_color_hex(0x333333);
}

int Gui::timeToMinutes(const String &hhmm)
{
    if (hhmm.length() < 5) return 0;
    return hhmm.substring(0, 2).toInt() * 60 + hhmm.substring(3, 5).toInt();
}

bool Gui::isCurrentEvent(const String &startISO, const String &endISO)
{
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo))
        return false;
    time_t now = mktime(&timeInfo);

    struct tm s = {}, e = {};
    s.tm_year = startISO.substring(0, 4).toInt() - 1900;
    s.tm_mon  = startISO.substring(5, 7).toInt() - 1;
    s.tm_mday = startISO.substring(8, 10).toInt();
    s.tm_hour = startISO.length() >= 16 ? startISO.substring(11, 13).toInt() : 0;
    s.tm_min  = startISO.length() >= 16 ? startISO.substring(14, 16).toInt() : 0;

    e.tm_year = endISO.substring(0, 4).toInt() - 1900;
    e.tm_mon  = endISO.substring(5, 7).toInt() - 1;
    e.tm_mday = endISO.substring(8, 10).toInt();
    e.tm_hour = endISO.length() >= 16 ? endISO.substring(11, 13).toInt() : 23;
    e.tm_min  = endISO.length() >= 16 ? endISO.substring(14, 16).toInt() : 59;

    return (now >= mktime(&s) && now <= mktime(&e));
}
