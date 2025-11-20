/**
 **************************************************
 *
 * @file        PartialUpdate.ino
 * @brief       Example showing how to use partial update on the Inkplate10
 *
 * For info on how to quickly get started with Inkplate 10 visit https://soldered.com/documentation/inkplate/10/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display in 1-bit mode
Inkplate inkplate(INKPLATE_1BIT);

// Timer ISR
esp_timer_handle_t lvgl_tick_timer;

//  ESP32 hardware timer that ticks the LVGL's millisecond tick counter
void lv_tick_task(void *arg){
  lv_tick_inc(5);
}

// Global definitions for LVGL objects
lv_obj_t *screen;
lv_obj_t *square;

unsigned long lastMoveTime=0;
int step=0;

void setup() {
  //Initialize the display 
  Serial.begin(115200);
  Serial.print("Inkplate LGVL partial update example.");
  inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
  inkplate.clearDisplay();
  inkplate.enableDithering(0);
  inkplate.clearDisplay();

  //Create a FreeRTOS timer that calls lv_tick_task() every 5 milliseconds
  const esp_timer_create_args_t lvgl_tick_args = {
    .callback = &lv_tick_task,
    .arg = nullptr,
    .dispatch_method = ESP_TIMER_TASK,
    .name = "lvgl_tick"
  };
  esp_timer_create(&lvgl_tick_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, 5000);

  // Screen setup
  screen = lv_scr_act(); //Get the active screen
  lv_disp_load_scr(screen); // Load the active screen for rendering
  lv_obj_set_style_bg_color(screen, lv_color_white(), 0); //Set background color
  lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0); // Set background as opaque

  // Create a square lvgl object
  square = lv_obj_create(screen); // Create a base object
  lv_obj_set_size(square, 50, 50); // Define object's dimensions
  lv_obj_set_style_bg_opa(square, LV_OPA_TRANSP, 0); //Set the object's background as opaque and transparent
  lv_obj_set_style_border_width(square, 4, 0); // Set the width of the objects border 
  lv_obj_set_style_border_color(square, lv_color_black(), 0); // Set the color of the object's border
  lv_obj_set_pos(square, 0, 0); // Set the initial position of the object

  // Initial render
  lv_timer_handler(); // Process LVGL's drawing pieine once, rendering the first frame

  lastMoveTime=millis();
  Serial.println("Setup complete");
}

void loop() {
  unsigned long currentTime = millis();

  // Save previous coordinates
  lv_area_t old_area;
  lv_obj_get_coords(square, &old_area);

  int newX = (step * 20)%(1200-50);
  int newY = (step * 20)%(825-50);

  //Move the square
  lv_obj_set_pos(square, newX, newY);

  // Get new coordinates for invalidation
  lv_area_t new_area;
  lv_obj_get_coords(square, &new_area);

  step++;
  lastMoveTime = currentTime;

  // Proces LVGV tasks, this will automatically trigger flush callbacks
  lv_timer_handler();

  Serial.println("Time it took to render in fb: " +String(millis()-currentTime)+"ms");
  inkplate.partialUpdate(0,1);
  delay(10); // Small delay to prevent overwhelming the system

}
