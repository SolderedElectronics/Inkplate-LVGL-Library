/**
 **************************************************
 *
 * @file        BurnInClean.ino
 * @brief       This example will try to remove heavy burn-in visible on the panel.
 *              Set number of refresh / clear cycles and upload the program.
 *
 * For info on how to quickly get started with Inkplate 6 visit
 * https://soldered.com/documentation/inkplate/6/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

// Next 3 lines are a precaution, you can ignore those, and the example would also work without them
#ifndef ARDUINO_INKPLATE6V2
#error "Wrong board selection for this example, please select Soldered Inkplate 6"
#endif

// Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate object and set it to work in monochrome mode
Inkplate inkplate(INKPLATE_1BIT);

// Number of clear cycles
#define CLEAR_CYCLES 20

// Delay between clear cycles (in milliseconds)
#define CYCLES_DELAY 5000

void setup() 
{
  inkplate.begin();         // Init library (you should call this function ONLY ONCE)
  inkplate.clearDisplay();  // Clear any data that may have been in (software) frame buffer

  inkplate.einkOn();        // Turn on epaper display

  int cycles = CLEAR_CYCLES;

  // Clean it by writing clear sequence to the panel
  while (cycles)
  {
    inkplate.clean(1, 15);
    inkplate.clean(2, 1);
    inkplate.clean(0, 5);
    inkplate.clean(2, 1);
    inkplate.clean(1, 15);
    inkplate.clean(2, 1);
    inkplate.clean(0, 5);
    inkplate.clean(2, 1);

    delay(CYCLES_DELAY);
    cycles--;
  }

  /* Display text when finished */
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Clearing Done!");
  lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 10);

  // Tick the LVGL timer by 50 
  lv_tick_inc(50);

  // Handle the new label and write it into the framebuffer
  lv_timer_handler();

  // Display the created label onto the screen
  inkplate.display();
}

void loop() 
{
  // Empty loop
}
