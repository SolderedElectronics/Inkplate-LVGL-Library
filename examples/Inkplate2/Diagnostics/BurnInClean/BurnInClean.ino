/*
   Inkplate2_Burn_In_Clean example for Soldered Inkplate 2
   For this example you will need only USB cable and Inkplate 2.
   Select "Soldered Inkplate2" from Tools -> Board menu.
   Don't have "Soldered Inkplate2" option? Follow our tutorial and add it:
   https://soldered.com/learn/add-inkplate-6-board-definition-to-arduino-ide/

   This example will try to remove heavy burn-in visible on the panel.
   Set number of refresh / clear cycles and upload the program.

   Want to learn more about Inkplate? Visit www.inkplate.io
   Looking to get support? Write on our forums: https://forum.soldered.com/
   12 January 2022 by e-radionica.com
*/



#include "Inkplate-LVGL.h" //Include Inkplate library to the sketch
Inkplate display;     // Create object on Inkplate library

// Nubmer of clear cycles.
#define CLEAR_CYCLES 5

// Delay between clear cycles (in milliseconds)
#define CYCLES_DELAY 5000

void setup()
{
    display.begin(LV_DISP_RENDER_MODE_FULL);        // Init library (you should call this function ONLY ONCE)
    display.clearDisplay(); // Clear any data that may have been in (software) frame buffer.

    int cycles = CLEAR_CYCLES;  // Set the number of clear cycles

    // Clean it by writing clear sequence to the panel.
    while (cycles)
    {
        cycles--;
        display.display();
        delay(CYCLES_DELAY);
    }

    // Display text when finished
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Clearing Done!");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_14, 0);
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 10);

    // Tick the LVGL timer a bit
    lv_tick_inc(50);

    // Handle the new label and write it into the framebuffer
    lv_timer_handler();

    // Display to the screen
    display.display();

}

void loop()
{
    // Empty...
}