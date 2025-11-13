/**
 **************************************************
 *
 * @file        BluetoothSerial.ino
 * @brief       Example showing how to use Bluetooth Serial on Inkplate 5V2
 *              with an LVGL-based UI that safely renders incoming text.
 *
 * HOW TO USE
 *  - Upload this example to Inkplate 5V2.
 *  - Pair your phone with the device in Bluetooth settings (device name: "Inkplate6").
 *  - Open a Serial Bluetooth Terminal app (or similar) and connect to "Inkplate6".
 *  - Anything you type in the app will be shown on the Inkplate screen.
 *  - Anything you type in the Arduino Serial Monitor will be sent to the phone.
 *
 *
 * For setup instructions and more information about Inkplate 5V2, visit:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 **************************************************
 */

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE5V2
#error "Wrong board selection for this example, please select Soldered Inkplate5 V2 in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <BluetoothSerial.h>

static const char *btDeviceName = "Inkplate6";

Inkplate inkplate(INKPLATE_1BIT);
BluetoothSerial SerialBT;

// ---------- Font helpers (use LV_FONT_DEFAULT only) ----------
static const lv_font_t* font_big()   { return LV_FONT_DEFAULT; }
static const lv_font_t* font_small() { return LV_FONT_DEFAULT; }

// ---------- LVGL helpers ----------
static inline int dispW() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_horizontal_resolution(d);
}
static inline int dispH() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_vertical_resolution(d);
}

static int lineHeight(const lv_font_t *f) {
  #if LVGL_VERSION_MAJOR >= 9
    return lv_font_get_line_height(f);
  #else
    // Approximate default font line height for LVGL v8 if API is unavailable
    return 18;
  #endif
}

static void make_opaque(lv_obj_t *obj) {
  lv_obj_set_style_bg_color(obj, lv_color_hex(0xFFFFFF), 0);
  lv_obj_set_style_bg_opa(obj, LV_OPA_COVER, 0);
  lv_obj_set_style_border_width(obj, 0, 0);
}

// ---------- UI elements ----------
static lv_obj_t *label_title = nullptr;
static lv_obj_t *label_info  = nullptr;
static lv_obj_t *panel_log   = nullptr;
static lv_obj_t *label_log   = nullptr;

// Spacing
static const int INFO_LINE_SPACE = 4;
static const int LOG_LINE_SPACE  = 6;
static const int topMargin  = 8;
static const int leftMargin = 8;
static const int rightMargin = 8;

// Log state
static String logText;
static int maxLines  = 0;
static int usedLines = 0;

static void ui_create() {
  // Screen background
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // Title
  label_title = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_title, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_title, font_big(), 0);
  lv_label_set_text(label_title, "Bluetooth Serial Example");
  lv_obj_set_pos(label_title, leftMargin, topMargin);
  make_opaque(label_title);

  // Instructions
  label_info = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_info, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_info, font_small(), 0);
  lv_label_set_long_mode(label_info, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_line_space(label_info, INFO_LINE_SPACE, 0);
  int infoY = topMargin + lineHeight(font_big()) + 8;
  lv_obj_set_pos(label_info, leftMargin, infoY);
  lv_obj_set_width(label_info, dispW() - leftMargin - rightMargin);
  lv_label_set_text(label_info,
    "Pair your phone and open a Serial Bluetooth Terminal.\n"
    "Messages from phone appear below.\n"
    "USB Serial -> phone is also forwarded.\n");
  make_opaque(label_info);

  // Ensure layout is computed before measuring info height
  lv_timer_handler();
  #if LVGL_VERSION_MAJOR >= 9
    lv_obj_update_layout(label_info);
  #endif
  int infoBottom = lv_obj_get_y(label_info) + lv_obj_get_height(label_info);

  // Opaque log panel below the instructions
  panel_log = lv_obj_create(lv_screen_active());
  make_opaque(panel_log);
  lv_obj_set_style_pad_all(panel_log, 0, 0);
  lv_obj_set_style_border_width(panel_log, 0, 0);
  int panelX = leftMargin;
  int panelY = infoBottom + 10;
  int panelW = dispW() - leftMargin - rightMargin;
  int panelH = dispH() - panelY - topMargin;
  lv_obj_set_pos(panel_log, panelX, panelY);
  lv_obj_set_size(panel_log, panelW, panelH);

  // Log label inside the panel
  label_log = lv_label_create(panel_log);
  lv_obj_set_style_text_color(label_log, lv_color_hex(0x000000), 0);
  lv_obj_set_style_text_font(label_log, font_small(), 0);
  lv_label_set_long_mode(label_log, LV_LABEL_LONG_WRAP);
  lv_obj_set_style_text_line_space(label_log, LOG_LINE_SPACE, 0);
  lv_obj_set_pos(label_log, 0, 0);
  lv_obj_set_width(label_log, panelW);

  // Compute how many lines fit into the panel
  int effLineH = lineHeight(font_small()) + LOG_LINE_SPACE;
  maxLines = (effLineH > 0) ? (panelH / effLineH) : 0;
  if (maxLines < 3) maxLines = 3; // safety minimum
}

static void ui_reset_log_full_refresh() {
  // Rebuild the entire UI and perform a full refresh (avoids ghosting)
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

  if (c == '\n') {
    logText += '\n';
    usedLines++;
  } else {
    logText += c;
  }

  if (usedLines >= maxLines) {
    ui_reset_log_full_refresh();
    return;
  }

  lv_label_set_text(label_log, logText.c_str());

  // Partial refresh for faster updates; opaque panel removes old pixels
  lv_tick_inc(5);
  lv_timer_handler();
  inkplate.partialUpdate();
}

void setup() {
  Serial.begin(115200);
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);

  ui_create();

  // Initial full refresh to clear any boot artifacts
  lv_tick_inc(20);
  lv_timer_handler();
  inkplate.display();

  // Start Bluetooth
  if (!SerialBT.begin(btDeviceName)) {
    const char *err = "Bluetooth init ERROR";
    for (const char *p = err; *p; ++p) log_append_char(*p);
    log_append_char('\n');
  } else {
    const char *ok1 = "BT ready. Pair and connect.";
    for (const char *p = ok1; *p; ++p) log_append_char(*p);
    log_append_char('\n');
    log_append_char('\n'); // extra spacing before user messages
  }
}

void loop() {
  // USB Serial -> Bluetooth
  while (Serial.available()) {
    char c = (char)Serial.read();
    SerialBT.write((uint8_t)c);
  }

  // Bluetooth -> screen + echo to USB Serial
  while (SerialBT.available()) {
    char c = (char)SerialBT.read();
    Serial.write((uint8_t)c);
    log_append_char(c);
  }

  lv_tick_inc(5);
  lv_timer_handler();
  delay(20);
}
