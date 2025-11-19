/**
 **************************************************
 *
 * @file        SimpleTouchscreen.ino
 * @brief       This example shows you how to use Inkplate 6 FLICK touchscreen.
 *              Once the code is uploaded, try to touch the rectangles on the screen :)
 *
 * For info on how to quickly get started with Inkplate 6FLICK visit https://soldered.com/documentation/inkplate/6flick/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

 // Include the Inkplate LVGL Library
#include <Inkplate-LVGL.h>

// Create an instance of Inkplate display in 1-bit mode (change to INKPLATE_3BIT if you want grayscale)
Inkplate inkplate(INKPLATE_1BIT);

// Create an lvgl task which will tick the lvgl timer every 5 ms
// and handle any animations needed
void lvgl_task(void *arg) 
{
  for (;;) 
  {
    lv_tick_inc(5);
    lv_timer_handler();
    vTaskDelay(pdMS_TO_TICKS(5));  
  }
}

// Initialize global lvgl rectangle object
static lv_obj_t *rect = NULL;

// Touch detect flag
bool isRectangleClicked = false;

// Rectangle coordinates
int x_position = 50;
int y_position = 50;

static void btn_event_cb(lv_event_t *e)
{
    lv_event_code_t code = lv_event_get_code(e);

    if (code == LV_EVENT_CLICKED) 
    {
        if (rect)
        {
            x_position += 100;
            y_position += 100;

            if (y_position < 660) 
            {
                lv_obj_set_pos(rect, x_position, y_position);
                lv_obj_invalidate(rect);
                isRectangleClicked = true;
            }
            else
            {
                x_position = 50;
                y_position = 50;

                inkplate.clearDisplay();
                lv_obj_clean(lv_scr_act());
                lv_draw_initials();
                lv_obj_invalidate(lv_scr_act());
                for (int i = 0; i < 5; i++) 
                {
                    lv_timer_handler();
                    delay(10);
                }
                inkplate.display();
            }
        }
    }
}

static void lv_draw_initials()
{
    /* Create a black label, set its text and font and align it to the center */
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_label_set_text(label, "Touch the rectangles on screen!");
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);
    lv_obj_set_style_text_font(label,  &lv_font_montserrat_48, 0);
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, 0);

    /* Create a black rectangle */
    rect = lv_obj_create(lv_scr_act());
    lv_obj_set_style_bg_color(rect, lv_color_hex(0x000000), 0);
    lv_obj_set_size(rect, 100, 50);
    lv_obj_set_pos(rect, x_position, y_position);

    // Add event callback function
    lv_obj_add_event_cb(rect, btn_event_cb, LV_EVENT_ALL, NULL);
}

void setup()
{
    Serial.begin(115200);
    Serial.println("Inkplate 6 FLICK Touchscreen Example.");

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);

    if (inkplate.touchscreen.init(true))
    {
        Serial.println("Touchscreen initialized.");
    }
    else
    {
        Serial.println("Touchscreen initialization failed.");
        while (true);
    }

    // Create LVGL task on core 1 to run independently from the rest of the sketch
    xTaskCreatePinnedToCore(
      lvgl_task, // Function which will be pinned 
      "lvgl_tick", // Symbolic name
      16000,  // Stack depth       
      nullptr, // No parameters
      2, // Priority
      nullptr, // No buffer, it will be allocated dynamically
      1 // core used
    );

    lv_draw_initials();

    // Force initial render
    for (int i = 0; i < 5; i++) 
    {
        lv_timer_handler();
        delay(10);
    }
    // Display content from buffer
    inkplate.display();
}

void loop()
{
    if (isRectangleClicked)
    {
        inkplate.partialUpdate(0, 1);
        isRectangleClicked = false;
    }
}

