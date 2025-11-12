/**
 **************************************************
 *
 * @file        Grayscale.ino
 * @brief       Example showing off the Grayscale capability of the Inkplate10
 *
 * For info on how to quickly get started with Inkplate 10 visit https://soldered.com/documentation/inkplate/10/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display in 3-bit mode 
Inkplate inkplate(INKPLATE_3BIT);

void setup() {
  /* Initialize the display as well as LVGL itself in FULL render mode,
     other possibilities are PARTIAL (fastest) and DIRECT (not currently supported)
     NOTE: Dithering is only supported in FULL render mode   */
  inkplate.begin(LV_DISP_RENDER_MODE_FULL);
  // Change the active screen's background color to white
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  lv_obj_t *screen = lv_scr_act(); //Get the active screen
  lv_disp_load_scr(screen); // Load the active screen for rendering
  lv_obj_set_style_bg_color(screen, lv_color_white(), 0); //Set background color
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0); // Set background as opaque
  
  int screenW=1200;
  int screenH=825;
  int colWidth=screenW/8;

  for(int i=0;i<8;i++){
    lv_obj_t *rect=lv_obj_create(screen);
    lv_obj_set_size(rect, colWidth, screenH); // Define object's dimensions
    lv_obj_set_pos(rect, i*colWidth, 0); // Set the object's position

    int grayscaled = map(i,0,7,0,255); // Map values 0-7 to 0-255 range
    lv_color_t gray=lv_color_make(grayscaled, grayscaled, grayscaled); // Set the gray color

    // Set background color (filled, with no border)
    lv_obj_set_style_bg_color(rect,gray,0); // Set background as opaque
    lv_obj_set_style_bg_opa(rect, LV_OPA_COVER, 0); //Set the rectangle's background as opaque and transparent
    lv_obj_set_style_border_width(rect, 0, 0); //Set the width of the rectangle's border
  }
  lv_timer_handler();
  inkplate.display();
}

void loop() {
  // Stays empty, label is only rendered once to the screen
}
