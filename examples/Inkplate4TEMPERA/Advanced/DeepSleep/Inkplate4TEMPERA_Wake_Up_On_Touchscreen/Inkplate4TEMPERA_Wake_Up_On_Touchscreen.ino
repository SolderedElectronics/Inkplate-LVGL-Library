/**
 **************************************************
 * @file        Inkplate4TEMPERA_Wake_Up_On_Touchscreen_LVGL.ino
 * @brief       Wake Inkplate 4 TEMPERA from deep sleep and show wake reason with LVGL.
 **************************************************
 */

#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <esp_sleep.h>

#define uS_TO_S_FACTOR 1000000ULL
#define TIME_TO_SLEEP 30

Inkplate inkplate(INKPLATE_1BIT);

// Boot counter survives deep sleep
RTC_DATA_ATTR int bootCount = 0;

void showInfo(const char *wakeText)
{
    lv_obj_clean(lv_screen_active());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    lv_obj_t *title = lv_label_create(lv_screen_active());
    lv_label_set_text(title, "Inkplate 4 TEMPERA");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_28, 0);
    lv_obj_align(title, LV_ALIGN_TOP_MID, 0, 30);

    char bootBuf[64];
    snprintf(bootBuf, sizeof(bootBuf), "Boot count: %d", bootCount);

    lv_obj_t *bootLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(bootLabel, bootBuf);
    lv_obj_set_style_text_font(bootLabel, &lv_font_montserrat_24, 0);
    lv_obj_align(bootLabel, LV_ALIGN_TOP_LEFT, 30, 110);

    lv_obj_t *wakeLabel = lv_label_create(lv_screen_active());
    lv_obj_set_width(wakeLabel, 520);
    lv_label_set_long_mode(wakeLabel, LV_LABEL_LONG_WRAP);
    lv_label_set_text(wakeLabel, wakeText);
    lv_obj_set_style_text_font(wakeLabel, &lv_font_montserrat_20, 0);
    lv_obj_align(wakeLabel, LV_ALIGN_TOP_LEFT, 30, 170);

    lv_obj_t *hintLabel = lv_label_create(lv_screen_active());
    lv_label_set_text(hintLabel, "Touch screen or press wake button.");
    lv_obj_set_style_text_font(hintLabel, &lv_font_montserrat_18, 0);
    lv_obj_align(hintLabel, LV_ALIGN_BOTTOM_LEFT, 30, -30);

    for (int i = 0; i < 5; i++)
    {
        lv_timer_handler();
        delay(10);
    }

    lv_timer_handler();
    inkplate.display();
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);

    if (inkplate.touchscreen.init(true))
    {
        Serial.println("Touchscreen init OK");
    }
    else
    {
        Serial.println("Touchscreen init failed!");
        while (true)
            ;
    }

    bootCount++;

    esp_sleep_wakeup_cause_t cause = esp_sleep_get_wakeup_cause();
    const char *wakeText = "Wakeup was not caused by deep sleep";

    switch (cause)
    {
    case ESP_SLEEP_WAKEUP_EXT0:
        wakeText = "Wakeup caused by wake button (EXT0 / GPIO36)";
        break;
    case ESP_SLEEP_WAKEUP_EXT1:
        wakeText = "Wakeup caused by external interrupt (EXT1 / touchscreen INT)";
        break;
    case ESP_SLEEP_WAKEUP_TIMER:
        wakeText = "Wakeup caused by timer";
        break;
    case ESP_SLEEP_WAKEUP_ULP:
        wakeText = "Wakeup caused by ULP";
        break;
    default:
        wakeText = "Wakeup was not caused by deep sleep";
        break;
    }

    Serial.printf("Boot count: %d\n", bootCount);
    Serial.printf("Wake cause enum: %d\n", (int)cause);

    showInfo(wakeText);

    delay(2000);

    // Timer fallback
    esp_sleep_enable_timer_wakeup(TIME_TO_SLEEP * uS_TO_S_FACTOR);

    // Wake button
    esp_sleep_enable_ext0_wakeup(GPIO_NUM_36, LOW);

    // Touchscreen interrupt line through board logic
    // Try LOW level first; on many touch IRQ lines the interrupt is active LOW
    esp_sleep_enable_ext1_wakeup((1ULL << GPIO_NUM_34), ESP_EXT1_WAKEUP_ALL_LOW);

    Serial.println("Entering deep sleep...");
    delay(100);

    esp_deep_sleep_start();
}

void loop()
{
}