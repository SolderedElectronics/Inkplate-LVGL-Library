/*
  Inkplate6 — Bluetooth Serial (LVGL verzija) — font-safe
  Automatski bira dostupne Montserrat fontove iz lv_conf.h
*/

#if !defined(ARDUINO_ESP32_DEV) && !defined(ARDUINO_INKPLATE6V2)
#error "Wrong board selection for this example, please select e-radionica Inkplate6 or Soldered Inkplate6 in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <BluetoothSerial.h>

// Promijeni po želji BT ime
static const char *btDeviceName = "Inkplate6";

// Inkplate-LVGL (1-bit, parcijalni update friendly)
Inkplate inkplate(INKPLATE_1BIT);
BluetoothSerial SerialBT;

// ---------------- Font helperi (biraju prvi dostupni) ----------------
static const lv_font_t* font_big() {
  // prefer: 36, 32, 28, 26, 24
  #if LV_FONT_MONTSERRAT_36
    return &lv_font_montserrat_36;
  #elif LV_FONT_MONTSERRAT_32
    return &lv_font_montserrat_32;
  #elif LV_FONT_MONTSERRAT_28
    return &lv_font_montserrat_28;
  #elif LV_FONT_MONTSERRAT_26
    return &lv_font_montserrat_26;
  #elif LV_FONT_MONTSERRAT_24
    return &lv_font_montserrat_24;
  #else
    return LV_FONT_DEFAULT;
  #endif
}
static const lv_font_t* font_small() {
  // prefer: 20, 18, 16, 14
  #if LV_FONT_MONTSERRAT_20
    return &lv_font_montserrat_20;
  #elif LV_FONT_MONTSERRAT_18
    return &lv_font_montserrat_18;
  #elif LV_FONT_MONTSERRAT_16
    return &lv_font_montserrat_16;
  #elif LV_FONT_MONTSERRAT_14
    return &lv_font_montserrat_14;
  #else
    return LV_FONT_DEFAULT;
  #endif
}

// ---- LVGL pomoćne stvari ----
static inline int dispW() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_horizontal_resolution(d);
}
static inline int dispH() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_vertical_resolution(d);
}

// UI elementi
static lv_obj_t *label_title = nullptr;
static lv_obj_t *label_info  = nullptr;
static lv_obj_t *label_log   = nullptr;

// Log state
static String logText;
static int    maxLines   = 0;
static int    usedLines  = 0;
static int    topMargin  = 8;
static int    leftMargin = 8;
static int    logY       = 0;

static int lineHeight(const lv_font_t *f) {
  #if LVGL_VERSION_MAJOR >= 9
    return lv_font_get_line_height(f);
  #else
    return 20; // približno za manje fontove
  #endif
}

static void ui_create() {
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  label_title = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_title, font_big(), 0);
  lv_label_set_text(label_title, "Bluetooth Serial Example");
  lv_obj_set_pos(label_title, leftMargin, topMargin);

  label_info = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_info, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_info, font_small(), 0);
  lv_label_set_long_mode(label_info, LV_LABEL_LONG_WRAP);
  int infoY = topMargin + lineHeight(font_big()) + 6;
  lv_obj_set_pos(label_info, leftMargin, infoY);
  lv_obj_set_width(label_info, dispW() - 2*leftMargin);
  lv_label_set_text(label_info,
    "Pair your phone and open a serial BT terminal.\n"
    "Messages from phone appear here.\n"
    "USB Serial -> phone is also forwarded.\n");

  label_log = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_log, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_log, font_small(), 0);
  lv_label_set_long_mode(label_log, LV_LABEL_LONG_WRAP);
  logY = infoY + lv_obj_get_height(label_info) + 8;
  lv_obj_set_pos(label_log, leftMargin, logY);
  lv_obj_set_width(label_log, dispW() - 2*leftMargin);

  int availH = dispH() - logY - topMargin;
  maxLines   = availH / lineHeight(font_small());
  if (maxLines < 3) maxLines = 3;
}

static void ui_reset_log_full_refresh() {
  lv_obj_clean(lv_screen_active());
  logText  = "";
  usedLines = 0;
  ui_create();

  lv_tick_inc(10);
  lv_timer_handler();
  inkplate.display(); // FULL
}

static void log_append_char(char c) {
  if (c == '\r') return;
  if (c == '\n') { logText += '\n'; usedLines++; }
  else           { logText += c; }

  if (usedLines >= maxLines) { ui_reset_log_full_refresh(); return; }

  lv_label_set_text(label_log, logText.c_str());
  lv_tick_inc(5);
  lv_timer_handler();
  inkplate.partialUpdate();
}

void setup() {
  Serial.begin(115200);
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);

  ui_create();
  lv_tick_inc(20);
  lv_timer_handler();
  inkplate.display(); // prvi FULL

  // Bluetooth
  if (!SerialBT.begin(btDeviceName)) {
    const char *err = "Bluetooth init ERROR";
    for (const char *p = err; *p; ++p) log_append_char(*p);
    log_append_char('\n');
  } else {
    const char *ok1 = "BT ready. Pair and connect.";
    for (const char *p = ok1; *p; ++p) log_append_char(*p);
    log_append_char('\n');
  }
}

void loop() {
  while (Serial.available()) {
    char c = (char)Serial.read();
    SerialBT.write((uint8_t)c);
  }

  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    Serial.write((uint8_t)c);
    log_append_char(c);
  }

  lv_tick_inc(5);
  lv_timer_handler();
  delay(20);
}
