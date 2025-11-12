// Inkplate 6 + LVGL — Grayscale demo
// Prikazuje 8 nijansi sive (3-bit mod, FULL render)

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_3BIT);

static inline int dispW() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_horizontal_resolution(d);
}
static inline int dispH() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_vertical_resolution(d);
}

void drawGrayscaleBars() {
  int w = dispW();
  int h = dispH();
  int colW = w / 8;

  for (int i = 0; i < 8; i++) {
    uint8_t shade = i * 36; // 0..255 u 8 koraka
    lv_obj_t *rect = lv_obj_create(lv_screen_active());
    lv_obj_remove_flag(rect, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_set_size(rect, colW, h);
    lv_obj_set_pos(rect, i * colW, 0);
    lv_obj_set_style_border_width(rect, 0, LV_PART_MAIN);
    lv_obj_set_style_bg_color(rect, lv_color_make(shade, shade, shade), LV_PART_MAIN);
  }

  // Legende 0-7
  for (int i = 0; i < 8; i++) {
    char txt[4];
    sprintf(txt, "%d", i);
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, lv_color_hex(0x000000), 0);
    lv_obj_align(label, LV_ALIGN_TOP_LEFT, i * colW + colW / 2 - 5, 10);
  }
}

void setup() {
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.selectDisplayMode(INKPLATE_3BIT);

  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  drawGrayscaleBars();

  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();
}

void loop() {}
