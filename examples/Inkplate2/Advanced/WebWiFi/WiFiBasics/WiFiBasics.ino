/**
 **************************************************
 *
 * @file        WiFiBasics.ino
 * @brief       Example showing how to connect to WiFi on the Inkplate 2 and print the status on 
 *              the serial monitor
 *
 * For info on how to quickly get started with Inkplate 2 visit https://soldered.com/documentation/inkplate/2/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>  // Include the Inkplate + LVGL library
#include <WiFi.h>            // Include the ESP32 WiFi library

// Create Inkplate instance
Inkplate inkplate;

// WiFi credentials (replace with your own)
const char *ssid = "your ssid";
const char *pass = "your password";

void setup() {
  // Start serial communication for debugging
  Serial.begin(115200);
  Serial.println("Inkplate WiFi connection example using LVGL...");

  /* Initialize the Inkplate display and LVGL in PARTIAL render mode.
     - LV_DISP_RENDER_MODE_FULL : full refresh (slower)
     - LV_DISP_RENDER_MODE_PARTIAL : partial updates (faster)
     - LV_DISP_RENDER_MODE_DIRECT : not supported yet
     Dithering is turned off to simplify rendering. */
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
  inkplate.clearDisplay();      // Clear the screen
  inkplate.enableDithering(false);  // Disable dithering for simplicity

  // Set up LVGL UI screen
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);  // White background

  // Create a label widget on the active screen
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label, lv_color_black(), 0);        // Black text color
  lv_obj_set_style_text_font(label, &lv_font_montserrat_8, 0);   // Set font size
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);                     // Center label on screen

  // Draw the initial label into the framebuffer
  lv_timer_handler();   // Process LVGL tasks and render the label

  // Begin connecting to WiFi
  WiFi.begin(ssid, pass);
  Serial.print("Connecting to WiFi");

  // Loop until WiFi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);                          // Wait half a second
    Serial.print(".");  // Print progress to Serial Monitor
  }

  // Once connected to WiFi:
  Serial.println("\nConnected to WiFi!");
  lv_label_set_text(label, "Connected!");  // Update label text to show success
  lv_timer_handler();                      // Render new text
  inkplate.display();                      // Full screen refresh for final result
}

void loop() {
  // Nothing to do — display stays static after connection
}