/**
 **************************************************
 *
 * @file        OpenWeatherColor.ino
 * @brief       Colorful OpenWeather example for Inkplate 13 SPECTRA (LVGL RGB565 + dithering).
 *
 * For info on how to quickly get started with Inkplate 13 SPECTRA visit:
 * https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

// Use full RGB565 color space for LVGL
#define USE_COLOR_IMAGE

#include <Inkplate-LVGL.h>

static const lv_color_t COLOR_BLACK = lv_color_hex(0x000000);
static const lv_color_t COLOR_WHITE = lv_color_hex(0xFFFFFF);
static const lv_color_t COLOR_YELLOW = lv_color_hex(0xFFFF00);
static const lv_color_t COLOR_RED = lv_color_hex(0xFF0000);
static const lv_color_t COLOR_BLUE = lv_color_hex(0x0000FF);
static const lv_color_t COLOR_GREEN = lv_color_hex(0x00FF00);

// Create Inkplate instance (RGB565 mode when USE_COLOR_IMAGE is defined)
Inkplate inkplate;

// Simple data container
struct WeatherData
{
    float tempC = 0.0f;
    float feelsC = 0.0f;
    int humidity = 0;
    int precipitation = 0;
    float wind = 0.0f;
    String condition;
    String location;
    String dateLine;
};

static lv_obj_t *createCard(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t bg, int radius)
{
    lv_obj_t *card = lv_obj_create(parent);
    lv_obj_set_pos(card, x, y);
    lv_obj_set_size(card, w, h);
    lv_obj_set_style_bg_color(card, bg, 0);
    lv_obj_set_style_border_width(card, 0, 0);
    lv_obj_set_style_radius(card, radius, 0);
    lv_obj_set_style_pad_all(card, 0, 0);
    return card;
}

static lv_obj_t *createPanel(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t bg, int radius)
{
    lv_obj_t *card = createCard(parent, x, y, w, h, bg, radius);
    lv_obj_set_style_border_width(card, 2, 0);
    lv_obj_set_style_border_color(card, COLOR_BLACK, 0);
    return card;
}

static lv_obj_t *createRect(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t bg, int radius)
{
    lv_obj_t *rect = createCard(parent, x, y, w, h, bg, radius);
    return rect;
}

static lv_obj_t *createLabel(lv_obj_t *parent, const char *text, const lv_font_t *font, lv_color_t color, int x,
                             int y)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_pos(label, x, y);
    return label;
}

static lv_obj_t *createCircle(lv_obj_t *parent, int x, int y, int size, lv_color_t color)
{
    lv_obj_t *circle = lv_obj_create(parent);
    lv_obj_set_pos(circle, x, y);
    lv_obj_set_size(circle, size, size);
    lv_obj_set_style_bg_color(circle, color, 0);
    lv_obj_set_style_border_width(circle, 0, 0);
    lv_obj_set_style_radius(circle, LV_RADIUS_CIRCLE, 0);
    return circle;
}

static void drawCloudIcon(lv_obj_t *parent, int x, int y)
{
    createCircle(parent, x + 10, y + 30, 60, COLOR_WHITE);
    createCircle(parent, x + 45, y + 10, 70, COLOR_WHITE);
    createCircle(parent, x + 95, y + 30, 55, COLOR_WHITE);
    lv_obj_t *base = createCard(parent, x + 10, y + 60, 140, 50, COLOR_WHITE, 28);
    lv_obj_set_style_radius(base, 28, 0);
}

static void drawSunIcon(lv_obj_t *parent, int x, int y)
{
    createCircle(parent, x, y, 90, COLOR_YELLOW);
    createCircle(parent, x + 16, y + 16, 58, COLOR_RED);
}

static void createPillButton(lv_obj_t *parent, const char *text, int x, int y, int w, int h, lv_color_t bg,
                             lv_color_t fg)
{
    lv_obj_t *pill = createCard(parent, x, y, w, h, bg, h / 2);
    lv_obj_t *label = lv_label_create(pill);
    lv_label_set_text(label, text);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_18, 0);
    lv_obj_set_style_text_color(label, fg, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);
}

static void createForecastCard(lv_obj_t *parent, const char *day, const char *temp, int x, int y)
{
    lv_obj_t *card = createPanel(parent, x, y, 140, 220, COLOR_WHITE, 20);
    createLabel(card, day, &lv_font_montserrat_18, COLOR_BLACK, 20, 18);
    createCircle(card, 40, 60, 60, COLOR_YELLOW);
    createCircle(card, 70, 90, 36, COLOR_WHITE);
    createLabel(card, temp, &lv_font_montserrat_24, COLOR_BLACK, 20, 155);
}

static void drawWindIcon(lv_obj_t *parent, int x, int y)
{
    createRect(parent, x, y + 6, 52, 6, COLOR_BLACK, 3);
    createRect(parent, x + 6, y + 20, 44, 6, COLOR_BLACK, 3);
    createRect(parent, x + 12, y + 34, 36, 6, COLOR_BLACK, 3);
}

static void drawHumidityIcon(lv_obj_t *parent, int x, int y)
{
    createCircle(parent, x + 8, y, 18, COLOR_GREEN);
    createRect(parent, x + 12, y + 14, 10, 16, COLOR_GREEN, 4);
}

static void drawPressureIcon(lv_obj_t *parent, int x, int y)
{
    createCircle(parent, x, y, 22, COLOR_RED);
    createRect(parent, x + 6, y + 10, 24, 4, COLOR_BLACK, 2);
}

void renderWeather(const WeatherData &data)
{
    lv_obj_clean(lv_screen_active());

    int32_t screen_w = lv_display_get_horizontal_resolution(lv_disp_get_default());
    int32_t screen_h = lv_display_get_vertical_resolution(lv_disp_get_default());

    // Background
    lv_obj_set_style_bg_color(lv_screen_active(), COLOR_WHITE, LV_PART_MAIN);

    // Header row
    createLabel(lv_screen_active(), "Osijek", &lv_font_montserrat_48, COLOR_BLACK, 80, 40);
    createLabel(lv_screen_active(), "Weather • News • Calendar", &lv_font_montserrat_32, COLOR_BLACK, 320, 54);
    createLabel(lv_screen_active(), "April 24, 2024 | 10:45 AM", &lv_font_montserrat_32, COLOR_BLACK,
                screen_w - 520, 54);

    // Main weather card
    lv_obj_t *weatherCard = createPanel(lv_screen_active(), 80, 120, 820, 300, COLOR_WHITE, 20);
    drawSunIcon(weatherCard, 60, 60);
    drawCloudIcon(weatherCard, 120, 110);
    createLabel(weatherCard, "25 C", &lv_font_montserrat_48, COLOR_BLACK, 420, 70);
    createLabel(weatherCard, data.condition.c_str(), &lv_font_montserrat_24, COLOR_BLACK, 420, 140);
    createLabel(weatherCard, "Feels Like: 27 C", &lv_font_montserrat_20, COLOR_RED, 420, 190);

    // Weather stats strip
    lv_obj_t *stats = createPanel(lv_screen_active(), 80, 440, 820, 100, COLOR_WHITE, 20);
    drawWindIcon(stats, 24, 24);
    createLabel(stats, "Wind: 12 km/h", &lv_font_montserrat_22, COLOR_BLACK, 80, 30);
    drawHumidityIcon(stats, 280, 20);
    createLabel(stats, "Humidity: 55%", &lv_font_montserrat_22, COLOR_BLACK, 330, 30);
    drawPressureIcon(stats, 520, 24);
    createLabel(stats, "Pres: 1015 hPa", &lv_font_montserrat_22, COLOR_BLACK, 570, 30);

    // News card
    lv_obj_t *news = createPanel(lv_screen_active(), 940, 120, 520, 420, COLOR_RED, 20);
    createLabel(news, "Local News", &lv_font_montserrat_22, COLOR_WHITE, 30, 26);
    createRect(news, 30, 62, 460, 2, COLOR_WHITE, 0);
    createLabel(news, "City Festival Begins This Weekend!", &lv_font_montserrat_22, COLOR_YELLOW, 30, 90);
    createLabel(news, "Enjoy music, food, and fun\nactivities for the whole\nfamily!",
                &lv_font_montserrat_18, COLOR_WHITE, 30, 170);
    createRect(news, 30, 260, 460, 2, COLOR_WHITE, 0);
    createLabel(news, "New Museum Exhibit Opens Downtown", &lv_font_montserrat_20, COLOR_YELLOW, 30, 280);
    createLabel(news, "Free entry this Friday\nfrom 10 AM - 6 PM.",
                &lv_font_montserrat_18, COLOR_WHITE, 30, 350);

    // Forecast cards
    int fc_y = 580;
    createForecastCard(lv_screen_active(), "Thu", "28|16", 80, fc_y);
    createForecastCard(lv_screen_active(), "Fri", "22|14", 260, fc_y);
    createForecastCard(lv_screen_active(), "Sat", "24|15", 440, fc_y);
    createForecastCard(lv_screen_active(), "Sun", "19|11", 620, fc_y);


    // Calendar card
    lv_obj_t *calendar = createPanel(lv_screen_active(), 940, 560, 520, 420, COLOR_WHITE, 20);
    createLabel(calendar, "Calendar", &lv_font_montserrat_24, COLOR_BLACK, 30, 20);
    createRect(calendar, 30, 54, 460, 2, COLOR_BLACK, 0);
    createCircle(calendar, 30, 80, 14, COLOR_GREEN);
    createLabel(calendar, "9:00 AM | Team Meeting", &lv_font_montserrat_22, COLOR_BLACK, 60, 76);
    createCircle(calendar, 30, 130, 14, COLOR_BLUE);
    createLabel(calendar, "11:00 AM | Call with Client", &lv_font_montserrat_22, COLOR_BLACK, 60, 126);
    createCircle(calendar, 30, 180, 14, COLOR_RED);
    createLabel(calendar, "2:00 PM | Project Review", &lv_font_montserrat_22, COLOR_BLACK, 60, 176);
    createCircle(calendar, 30, 230, 14, COLOR_GREEN);
    createLabel(calendar, "4:00 PM | Gym Session", &lv_font_montserrat_22, COLOR_BLACK, 60, 226);
    createCircle(calendar, 30, 280, 14, COLOR_BLUE);
    createLabel(calendar, "6:30 PM | Dinner with Friends", &lv_font_montserrat_22, COLOR_BLACK, 60, 276);
    createCircle(calendar, 30, 330, 14, COLOR_RED);
    createLabel(calendar, "8:00 PM | Movie Night", &lv_font_montserrat_22, COLOR_BLACK, 60, 326);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Inkplate 13 SPECTRA weather dashboard mockup (LVGL RGB565)...");

    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.clearDisplay();
    inkplate.enableDithering(true);

    WeatherData data;
    data.location = "Osijek, Croatia";
    data.dateLine = "Tuesday, 20 Jan 2023";
    data.condition = "Partly Cloudy";
    data.tempC = 18.0f;
    data.feelsC = 19.0f;
    data.humidity = 77;
    data.precipitation = 93;
    data.wind = 6.0f;

    renderWeather(data);
    lv_timer_handler();
    inkplate.display();
}

void loop()
{
    delay(50);
}
