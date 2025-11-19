/**
 **************************************************
 *
 * @file        WifiBasics.ino
 * @brief       Example showing how to connect to WiFi on the Inkplate 5V2
 *
 * For setup instructions and more information about Inkplate 5V2, visit:
 * https://soldered.com/documentation/inkplate/5v2/overview/
 *
 * This example connects the Inkplate to WiFi and displays
 * connection status on the e-paper using LVGL.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>  // Include the Inkplate + LVGL library
#include <WiFi.h>           // Include the ESP32 WiFi library

// Create Inkplate instance in 1-bit mode (black & white only)
Inkplate inkplate(INKPLATE_1BIT);

// WiFi credentials (replace with your own)
const char *ssid = "your ssid";
const char *pass = "your password";

// LVGL tick timer handle for periodic updates
esp_timer_handle_t lvgl_tick_timer;

// ESP32 timer interrupt routine — increments LVGL’s internal tick every 5 ms
void lv_tick_task(void *arg) {
  lv_tick_inc(5);  // Increase LVGL's time base by 5 milliseconds
}

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("Inkplate WiFi connection example using LVGL...");

  /* Initialize the Inkplate display and LVGL in PARTIAL render mode.
     - LV_DISP_RENDER_MODE_FULL : full refresh (slower)
     - LV_DISP_RENDER_MODE_PARTIAL : partial updates (faster)
     - LV_DISP_RENDER_MODE_DIRECT : not supported yet
     Dithering is turned off because it's not needed in 1-bit mode. */
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
  inkplate.clearDisplay();          // Clear the screen
  inkplate.enableDithering(false);  // Disable dithering

  // Create and start a periodic LVGL tick timer (fires every 5 ms)
  const esp_timer_create_args_t lvgl_tick_args = {
      .callback = &lv_tick_task,           // Function to call every tick
      .arg = nullptr,                      // No arguments passed
      .dispatch_method = ESP_TIMER_TASK,   // Run in ESP32 timer task
      .name = "lvgl_tick"};                // Name of the timer for debugging
  esp_timer_create(&lvgl_tick_args, &lvgl_tick_timer);     // Create timer
  esp_timer_start_periodic(lvgl_tick_timer, 5000);         // Start (5000 µs = 5 ms)

  // Set up LVGL UI screen
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);  // White background

  // Create a label widget on the active screen
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_label_set_text(label, "Connecting to WiFi");                 // Initial text
  lv_obj_set_style_text_color(label, lv_color_black(), 0);        // Black text color
  lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);   // Set font size
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);                     // Center label on screen

  // Draw the initial label to the screen
  lv_timer_handler();   // Process LVGL tasks
  inkplate.display();   // Full display refresh

  // Begin connecting to WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");

  // For animated "..." text on screen
  String text = "Connecting to WiFi";
  int dotCount = 0;

  // Loop until WiFi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);                          // Wait half a second
    dotCount = (dotCount + 1) % 4;       // Cycle animation dots: 0–3

    // Build animated text
    text = "Connecting to WiFi";
    for (int i = 0; i < dotCount; i++)
      text += '.';

    // Update LVGL label text dynamically
    lv_label_set_text(label, text.c_str());
    lv_timer_handler();       // Process LVGL rendering
    inkplate.partialUpdate(); // Fast partial update

    Serial.print(".");        // Debug output
  }

  // Once connected to WiFi:
  Serial.println("\nConnected to WiFi!");
  lv_label_set_text(label, "Connected!");  // Update label text
  lv_timer_handler();                      // Redraw
  inkplate.display();                      // Full refresh for final result
}

void loop() {
  // Nothing to do — the e-paper display remains static after connection
}
