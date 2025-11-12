// Inkplate 6 + LVGL — Black & White demo
// Jednostavna podjela ekrana na crni i bijeli blok (1-bit mod)

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT);

static inline int dispW() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_horizontal_resolution(d);
}
static inline int dispH() {
  lv_display_t *d = lv_display_get_default();
  return lv_display_get_vertical_resolution(d);
}

void drawBlackWhite() {
  int w = dispW();
  int h = dispH();
  int half = w / 2;

  lv_obj_t *left = lv_obj_create(lv_screen_active());
  lv_obj_remove_flag(left, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(left, half, h);
  lv_obj_set_pos(left, 0, 0);
  lv_obj_set_style_border_width(left, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(left, lv_color_hex(0x000000), LV_PART_MAIN);

  lv_obj_t *right = lv_obj_create(lv_screen_active());
  lv_obj_remove_flag(right, LV_OBJ_FLAG_SCROLLABLE);
  lv_obj_set_size(right, w - half, h);
  lv_obj_set_pos(right, half, 0);
  lv_obj_set_style_border_width(right, 0, LV_PART_MAIN);
  lv_obj_set_style_bg_color(right, lv_color_hex(0xFFFFFF), LV_PART_MAIN);
}

void setup() {
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  inkplate.selectDisplayMode(INKPLATE_1BIT);

  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
  drawBlackWhite();

  lv_tick_inc(50);
  lv_timer_handler();
  inkplate.display();
}

void loop() {}
