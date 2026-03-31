/**
 **************************************************
 *
 * @file        HTTPGETRequest.ino
 * @brief       Example showing how to make an HTTP GET request and display the response on Inkplate 13SPECTRA
 *
 * For info on how to quickly get started with Inkplate 13SPECTRA visit https://soldered.com/documentation/inkplate/13-spectra/overview/
 *
 * This example connects to WiFi, fetches data from example.com,
 * and prints part of the response text on the e-paper screen.
 *
 * @authors     Soldered
 * @date        March 2026
 ***************************************************/

// Board selection safety check
#if !defined(ARDUINO_INKPLATE13SPECTRA)
#error "Wrong board selection for this example, please select Inkplate 13 (ESP32-S3) in the boards menu."
#endif

#define USE_COLOR_IMAGE
#include <Inkplate-LVGL.h>  // Include Inkplate + LVGL library for UI and display handling
#include <WiFi.h>           // ESP32 WiFi library for network connectivity
#include <HTTPClient.h>     // HTTP client library for GET/POST requests

// Create Inkplate instance
Inkplate inkplate;

// WiFi credentials (replace with your own network SSID and password)
const char *ssid = "your ssid";
const char *pass = "your password";

void setup()
{
    Serial.begin(115200);  // Initialize serial communication for debugging
    Serial.println("Inkplate HTTP GET request example using LVGL...");

    // Initialize Inkplate display in FULL render mode
    // - FULL: required for Inkplate 13SPECTRA, refreshes the entire screen
    inkplate.begin(LV_DISP_RENDER_MODE_FULL);
    inkplate.enableDithering(true);  // Enable dithering to extend visible color range

    // Set up LVGL screen (background and text elements)
    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_white(), LV_PART_MAIN);  // Set white background

    // Create a label in the center of the screen
    lv_obj_t *label = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_color(label, lv_color_black(), 0);       // Black text
    lv_obj_set_style_text_font(label, &lv_font_montserrat_28, 0);  // Medium-sized font
    lv_obj_align(label, LV_ALIGN_CENTER, 0, 0);                    // Center align label

    lv_tick_inc(50);
    lv_timer_handler();  // Process LVGL tasks and render

    // Start WiFi connection process
    WiFi.begin(ssid, pass);
    while (WiFi.status() != WL_CONNECTED)  // Wait until WiFi connects
    {
        delay(500);
        Serial.print(".");  // Print progress dots in Serial Monitor
    }

    // Once connected, update both serial and display
    Serial.println("\nConnected to WiFi!");
    lv_tick_inc(50);
    lv_timer_handler();

    // Create HTTPClient instance
    HTTPClient http;
    http.begin("https://example.com");  // Set target URL

    // Send HTTP GET request and store response code
    int httpCode = http.GET();

    // If request succeeds (positive response code)
    if (httpCode > 0)
    {
        Serial.printf("HTTP GET code: %d\n", httpCode);
        String payload = http.getString();  // Read server response body
        Serial.println("Response received!");

        // Clear previous content and display response text
        lv_obj_clean(lv_screen_active());                                   // Remove old label from screen
        lv_obj_t *responseLabel = lv_label_create(lv_screen_active());     // Create new label
        lv_label_set_text(responseLabel, payload.c_str());                  // Set text to response
        lv_obj_set_width(responseLabel, 1560);                              // Limit text width to screen
        lv_obj_set_style_text_color(responseLabel, lv_color_black(), 0);
        lv_obj_set_style_text_font(responseLabel, &lv_font_montserrat_22, 0);  // Font for long text
        lv_obj_align(responseLabel, LV_ALIGN_TOP_LEFT, 20, 20);            // Place label near top-left

        lv_tick_inc(50);
        lv_timer_handler();    // Process LVGL updates
        inkplate.display();    // Full update for clean final text
    }
    else
    {
        // If request fails, show error message
        Serial.printf("HTTP GET failed, code: %d\n", httpCode);
        lv_label_set_text(label, "HTTP GET failed!");
        lv_tick_inc(50);
        lv_timer_handler();
        inkplate.display();  // Full update for clear message
    }

    http.end();  // Close HTTP connection and free resources
}

void loop()
{
    // No loop logic needed — the result stays on the e-paper screen
}
