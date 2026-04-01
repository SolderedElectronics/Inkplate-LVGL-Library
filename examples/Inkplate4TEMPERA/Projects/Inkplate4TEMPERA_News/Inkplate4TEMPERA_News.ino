/**
 **************************************************
 * @file        Inkplate4TEMPERA_News_API_LVGL.ino
 * @brief       Fetch news headlines from NewsAPI.org over WiFi, render a
 *              "World News" layout with LVGL, then deep sleep between updates.
 **************************************************/

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

// ---------- CHANGE HERE -------------
int timeZone = 2;

char ssid[] = "your ssid";
char pass[] = "your password";
char api_key_news[] = "your api key";
// ------------------------------------

// Use Inkplate LVGL library
#include "Inkplate-LVGL.h"
#include "src/Network.h"
#include <time.h>

// Network object
NetworkFunctions network;

// Inkplate in 1-bit mode
Inkplate inkplate(INKPLATE_1BIT);

// Refresh interval
#define DELAY_MS (uint32_t)60 * 60 * 1000

// Layout constants for TEMPERA portrait-like UI
static const int SCREEN_W = 600;
static const int SCREEN_H = 600;

static const int PAGE_MARGIN = 24;
static const int CONTENT_W = SCREEN_W - (2 * PAGE_MARGIN);

// Function declarations
void setTime();
void buildNewsScreen(struct news *entities);
void buildErrorScreen(const char *msg);
lv_obj_t *createDivider(lv_obj_t *parent, int y, int thickness = 2);
lv_obj_t *createWrappedLabel(lv_obj_t *parent, const char *txt, int x, int y, int w, const lv_font_t *font);
void doFullRefresh();

void setup()
{
    Serial.begin(115200);
    delay(100);

    inkplate.begin();

    network.setCredentials(ssid, pass, api_key_news);
    network.setTimeZone(timeZone);
    network.begin();

    setTime();

    struct news *entities = network.getData();

    if (entities != nullptr)
    {
        buildNewsScreen(entities);
    }
    else
    {
        buildErrorScreen("Failed to fetch news");
    }

    doFullRefresh();

    esp_sleep_enable_timer_wakeup(1000ULL * DELAY_MS);
    esp_deep_sleep_start();
}

void loop()
{
    // Not used
}

/**
 * @brief Wait until valid NTP time is available and print current time to Serial.
 */
void setTime()
{
    struct tm timeInfo;
    time_t nowSec;

    inkplate.getNTPEpoch(&nowSec);
    while (nowSec < 8 * 3600 * 2)
    {
        delay(500);
        yield();
        nowSec = time(nullptr);
    }

    gmtime_r(&nowSec, &timeInfo);

    Serial.print(F("Current time: "));
    Serial.print(asctime(&timeInfo));
}

/**
 * @brief Create a horizontal black divider using LVGL.
 */
lv_obj_t *createDivider(lv_obj_t *parent, int y, int thickness)
{
    lv_obj_t *line = lv_obj_create(parent);
    lv_obj_remove_style_all(line);

    lv_obj_set_pos(line, PAGE_MARGIN, y);
    lv_obj_set_size(line, CONTENT_W, thickness);

    lv_obj_set_style_bg_color(line, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(line, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(line, 0, 0);
    lv_obj_set_style_radius(line, 0, 0);

    return line;
}

/**
 * @brief Create a wrapped label positioned absolutely.
 */
lv_obj_t *createWrappedLabel(lv_obj_t *parent, const char *txt, int x, int y, int w, const lv_font_t *font)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_long_mode(label, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(label, w);
    lv_label_set_text(label, txt);

    lv_obj_set_pos(label, x, y);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_bg_opa(label, LV_OPA_TRANSP, 0);

    return label;
}

/**
 * @brief Build the news page UI using LVGL objects only.
 */
void buildNewsScreen(struct news *entities)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_size(scr, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    // Title
    lv_obj_t *title = lv_label_create(scr);
    lv_label_set_text(title, "World News");
    lv_obj_set_style_text_color(title, lv_color_black(), 0);
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 12);

    createDivider(scr, 58, 3);

    // Time / date
    struct tm timeInfo;
    time_t nowSec;

    inkplate.getNTPEpoch(&nowSec);
    while (nowSec < 8 * 3600 * 2)
    {
        delay(500);
        yield();
        nowSec = time(nullptr);
    }
    gmtime_r(&nowSec, &timeInfo);

    char dateStr[32];
    char updateStr[32];

    snprintf(dateStr, sizeof(dateStr), "Date : %02d.%02d.%04d",
             timeInfo.tm_mday,
             timeInfo.tm_mon + 1,
             timeInfo.tm_year + 1900);

    snprintf(updateStr, sizeof(updateStr), "Last update : %02d:%02d",
             timeInfo.tm_hour,
             timeInfo.tm_min);

    lv_obj_t *dateLabel = lv_label_create(scr);
    lv_label_set_text(dateLabel, dateStr);
    lv_obj_set_style_text_font(dateLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(dateLabel, lv_color_black(), 0);
    lv_obj_align(dateLabel, LV_ALIGN_TOP_LEFT, 34, 70);

    lv_obj_t *updateLabel = lv_label_create(scr);
    lv_label_set_text(updateLabel, updateStr);
    lv_obj_set_style_text_font(updateLabel, &lv_font_montserrat_16, 0);
    lv_obj_set_style_text_color(updateLabel, lv_color_black(), 0);
    lv_obj_align(updateLabel, LV_ALIGN_TOP_RIGHT, -28, 70);

    createDivider(scr, 96, 3);

    // News container
    lv_obj_t *newsContainer = lv_obj_create(scr);
    lv_obj_remove_style_all(newsContainer);
    lv_obj_set_pos(newsContainer, 0, 112);
    lv_obj_set_size(newsContainer, SCREEN_W, SCREEN_H - 112);
    lv_obj_set_style_bg_opa(newsContainer, LV_OPA_TRANSP, 0);
    lv_obj_set_layout(newsContainer, LV_LAYOUT_FLEX);
    lv_obj_set_flex_flow(newsContainer, LV_FLEX_FLOW_COLUMN);
    lv_obj_set_style_pad_left(newsContainer, PAGE_MARGIN, 0);
    lv_obj_set_style_pad_right(newsContainer, PAGE_MARGIN, 0);
    lv_obj_set_style_pad_top(newsContainer, 8, 0);
    lv_obj_set_style_pad_bottom(newsContainer, 0, 0);
    lv_obj_set_style_pad_row(newsContainer, 10, 0);
    lv_obj_set_scrollbar_mode(newsContainer, LV_SCROLLBAR_MODE_OFF);

    // Approximate visible box count like original layout
    const int maxBoxes = 3;

    for (int i = 0; i < maxBoxes; i++)
    {
        if (entities[i].title == nullptr || entities[i].description == nullptr)
            break;

        lv_obj_t *box = lv_obj_create(newsContainer);
        lv_obj_set_width(box, CONTENT_W);
        lv_obj_set_height(box, 130);

        lv_obj_set_style_bg_color(box, lv_color_white(), 0);
        lv_obj_set_style_bg_opa(box, LV_OPA_COVER, 0);
        lv_obj_set_style_border_color(box, lv_color_black(), 0);
        lv_obj_set_style_border_width(box, 1, 0);
        lv_obj_set_style_border_side(box, LV_BORDER_SIDE_BOTTOM, 0);
        lv_obj_set_style_radius(box, 0, 0);
        lv_obj_set_style_pad_left(box, 0, 0);
        lv_obj_set_style_pad_right(box, 0, 0);
        lv_obj_set_style_pad_top(box, 0, 0);
        lv_obj_set_style_pad_bottom(box, 0, 0);
        lv_obj_set_scrollbar_mode(box, LV_SCROLLBAR_MODE_OFF);

        // Title
        lv_obj_t *headline = createWrappedLabel(
            box,
            entities[i].title,
            0,
            0,
            CONTENT_W - 20,
            &lv_font_montserrat_20
        );

        // Description
        lv_obj_t *desc = createWrappedLabel(
            box,
            entities[i].description,
            0,
            58,
            CONTENT_W - 50,
            &lv_font_montserrat_14
        );

        (void)headline;
        (void)desc;
    }

    lv_scr_load(scr);
}

/**
 * @brief Error screen using LVGL.
 */
void buildErrorScreen(const char *msg)
{
    lv_obj_t *scr = lv_obj_create(NULL);
    lv_obj_remove_style_all(scr);
    lv_obj_set_size(scr, SCREEN_W, SCREEN_H);
    lv_obj_set_style_bg_color(scr, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(scr, LV_OPA_COVER, 0);

    lv_obj_t *label = lv_label_create(scr);
    lv_label_set_text(label, msg);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_20, 0);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_center(label);

    lv_scr_load(scr);
}

/**
 * @brief Let LVGL render, then push full e-paper refresh.
 */
void doFullRefresh()
{
    lv_timer_handler();
    delay(20);
    lv_timer_handler();
    delay(20);

    inkplate.display();
}