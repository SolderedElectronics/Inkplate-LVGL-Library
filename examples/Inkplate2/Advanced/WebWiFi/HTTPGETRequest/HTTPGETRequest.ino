/**
 **************************************************
 *
 * @file        HTTPGETRequest.ino
 * @brief       Example showing how to make an HTTP GET request and display the response on Inkplate 2
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/2/overview/
 *
 * This example connects to WiFi, fetches data from example.com,
 * and prints part of the response text on the e-paper screen.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/
#include <Inkplate-LVGL.h>   // Include Inkplate + LVGL library for UI and display handling
#include <WiFi.h>            // ESP32 WiFi library for network connectivity
#include <HTTPClient.h>      // HTTP client library for GET/POST requests

// Create Inkplate instance 
Inkplate inkplate;

// WiFi credentials (replace with your own network SSID and password)
const char *ssid = "your ssid";
const char *pass = "your password";

// LVGL tick timer handle for periodic LVGL time updates
esp_timer_handle_t lvgl_tick_timer;

// LVGL tick task — runs every 5 ms to update LVGL's internal timing system
void lv_tick_task(void *arg) {
  lv_tick_inc(5);  // Increment LVGL’s tick counter by 5 milliseconds
}

void setup() {
  Serial.begin(115200);  // Initialize serial communication for debugging
  Serial.println("Inkplate HTTP GET request example using LVGL...");

  // Initialize Inkplate display in PARTIAL render mode
  // - PARTIAL: faster, updates only changed areas
  // - FULL: slower but redraws entire screen
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
  inkplate.clearDisplay();       // Clear any previous data from display memory
  inkplate.enableDithering(false); // Disable dithering (only useful in grayscale mode)

  // Create and start LVGL tick timer (runs every 5 ms)
  const esp_timer_create_args_t lvgl_tick_args = {
      .callback = &lv_tick_task,           // Function to call each tick
      .arg = nullptr,                      // No arguments passed
      .dispatch_method = ESP_TIMER_TASK,   // Runs as an ESP32 background task
      .name = "lvgl_tick"};                // Name for debugging
  esp_timer_create(&lvgl_tick_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, 5000); // 5000 µs = 5 ms tick interval

  // Set up LVGL screen (background and text elements)
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN); // Set white background

  // Create a label in the center of the screen
  lv_obj_t *label = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label, lv_color_black(), 0);  // Black text
  lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0); // Medium-sized font
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);  // Center align label

  lv_timer_handler();   // Process LVGL tasks and render

  // Start WiFi connection process
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {  // Wait until WiFi connects
    delay(500);
    Serial.print(".");                     // Print progress dots in Serial Monitor
  }

  // Once connected, update both serial and display
  Serial.println("\nConnected to WiFi!");
  lv_timer_handler();


  // Create HTTPClient instance
  HTTPClient http;
  http.begin("https://example.com");  // Set target URL

  // Send HTTP GET request and store response code
  int httpCode = http.GET();

  // If request succeeds (positive response code)
  if (httpCode > 0) {
    Serial.printf("HTTP GET code: %d\n", httpCode);
    String payload = http.getString();  // Read server response body
    Serial.println("Response received!");


    // Clear previous content and display response text
    lv_obj_clean(lv_screen_active());  // Remove old label from screen
    lv_obj_t *responseLabel = lv_label_create(lv_screen_active());  // Create new label
    lv_label_set_text(responseLabel, payload.c_str());              // Set text to response
    lv_obj_set_width(responseLabel, 212);                          // Limit text width to screen
    lv_obj_set_style_text_color(responseLabel, lv_color_black(), 0);
    lv_obj_set_style_text_font(responseLabel, &lv_font_montserrat_8, 0); // Smaller font for long text
    lv_obj_align(responseLabel, LV_ALIGN_TOP_LEFT, 20, 20);         // Place label near top-left

    lv_timer_handler();  // Process LVGL updates
    inkplate.display();  // Full update for clean final text
  } else {
    // If request fails, show error message
    Serial.printf("HTTP GET failed, code: %d\n", httpCode);
    lv_label_set_text(label, "HTTP GET failed!");
    lv_timer_handler();
    inkplate.display();  // Full update for clear message
  }

  http.end();  // Close HTTP connection and free resources
}

void loop() {
  // No loop logic needed — the result stays on the e-paper screen
}