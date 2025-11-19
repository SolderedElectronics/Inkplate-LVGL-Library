/**
 **************************************************
 *
 * @file        HTTPPOSTRequest.ino
 * @brief       Example showing how to make an HTTP POST request
 *              and display the response on Inkplate10 using LVGL.
 *
 * For info on how to quickly get started with Inkplate 6COLOR visit https://soldered.com/documentation/inkplate/6color/overview/
 *
 * This example connects to WiFi, sends a POST request to webhook.site,
 * and displays the server’s response on the Inkplate e-paper screen.
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************/

#include <Inkplate-LVGL.h>   // Include Inkplate + LVGL graphics library
#include <WiFi.h>            // Include ESP32 WiFi library
#include <HTTPClient.h>      // Include HTTPClient for HTTP POST requests

// Create Inkplate instance in 1-bit mode (black and white)
Inkplate inkplate;

// Wi-Fi credentials (replace with your own network info)
const char *ssid = "your ssid";
const char *pass = "your password";

// LVGL tick timer handle (used for regular LVGL updates)
esp_timer_handle_t lvgl_tick_timer;

// Tick handler for LVGL (called every 5 milliseconds)
// This keeps LVGL’s internal timing synchronized
void lv_tick_task(void *arg) {
  lv_tick_inc(5);  // Increment LVGL tick counter by 5 ms
}

void setup() {
  Serial.begin(115200);  // Initialize serial output for debugging
  Serial.println("Inkplate HTTP POST request example using LVGL...");

  // Initialize Inkplate in PARTIAL render mode
  //  - PARTIAL: faster, only updates changed screen regions
  //  - FULL: slower, refreshes entire display
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);
  inkplate.clearDisplay();        // Clear screen memory
  inkplate.enableDithering(false); // Disable dithering (not needed for 1-bit mode)

  // Create and start LVGL tick timer to run every 5 ms
  const esp_timer_create_args_t lvgl_tick_args = {
      .callback = &lv_tick_task,           // Function to call each tick
      .arg = nullptr,                      // No arguments passed
      .dispatch_method = ESP_TIMER_TASK,   // Run inside ESP32 timer task
      .name = "lvgl_tick"};                // Debug name for timer
  esp_timer_create(&lvgl_tick_args, &lvgl_tick_timer);
  esp_timer_start_periodic(lvgl_tick_timer, 5000); // 5000 µs = 5 ms period

  // Set up LVGL screen and a text label
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN); // White background

  lv_obj_t *label = lv_label_create(lv_screen_active()); // Create label widget
  lv_obj_set_style_text_color(label, lv_color_black(), 0);  // Black text
  lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0); // Medium font size
  lv_obj_align(label, LV_ALIGN_CENTER, 0, 0); // Center label on screen

  lv_timer_handler();   // Process LVGL drawing tasks

  // Wi-Fi Connection Section
  WiFi.begin(ssid, pass);         // Start Wi-Fi connection
  Serial.print("Connecting to WiFi");
  while (WiFi.status() != WL_CONNECTED) { // Wait until connected
    delay(500);
    Serial.print(".");             // Print progress indicator to Serial Monitor
  }
  Serial.println("\nConnected!");  // Connected successfully

  // Update label text to show connection success
  Serial.println("Connected to WiFi!");
  lv_timer_handler();
  delay(1000);              // Small pause before making POST request

  // Begin HTTP POST request
  Serial.println("Sending POST request...");
  lv_timer_handler();

  HTTPClient http; // Create HTTPClient instance

  // Replace with your personal unique webhook.site URL
  String url = "https://webhook.site/319722c5-2a7a-4333-92b3-2db3a194fb66";
  http.begin(url); // Set the target URL for the POST request

  // Set HTTP headers and body (JSON payload)
  http.addHeader("Content-Type", "application/json");
  String postData = "{\"device\":\"Inkplate6COLOR\", \"status\":\"Hello from Inkplate!\"}";

  // Send POST request and capture response code
  int httpCode = http.POST(postData);

  // --- Handle HTTP Response ---
  if (httpCode > 0) { // If HTTP response was received
    Serial.printf("HTTP POST code: %d\n", httpCode);
    String payload = http.getString(); // Read response body as string
    Serial.println("Response received!");

    // Clear previous UI and display server response text
    lv_obj_clean(lv_screen_active()); // Remove old label
    lv_obj_t *respLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(respLabel, payload.c_str()); // Set label to response text
    lv_obj_set_width(respLabel, 448);             // Limit text width to prevent overflow
    lv_obj_set_style_text_color(respLabel, lv_color_black(), 0);
    lv_obj_set_style_text_font(respLabel, &lv_font_montserrat_14, 0); // Smaller font for response
    lv_obj_align(respLabel, LV_ALIGN_TOP_LEFT, 20, 20); // Align text near top-left corner

    lv_timer_handler(); // Process LVGL updates
    inkplate.display(); // Perform full refresh for clean final screen
  } else {
    // If POST failed, show error message on both Serial and display
    Serial.printf("POST failed, code: %d\n", httpCode);
    lv_label_set_text(label, "POST failed!");
    lv_timer_handler();
    inkplate.display();
  }

  http.end(); // Close HTTP connection and free memory
}

void loop() {
  // Nothing to do in loop — e-paper retains image until next refresh
}