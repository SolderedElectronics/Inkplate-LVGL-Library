/**
 **************************************************
 * @file        Inkplate4TEMPERA_SD_TXT_Read.ino
 * @brief       Read and display text file from SD card on Soldered Inkplate 4 TEMPERA using LVGL.
 *
 * @details     Demonstrates how to open a .txt file from a FAT-formatted SD card
 *              and display its contents on the Inkplate 4 TEMPERA e-paper display
 *              using LVGL labels.
 *
 * @author      Soldered
 * @date        2026
 **************************************************/
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT);
SdFile file;

// LVGL labels
static lv_obj_t *lbl_status = nullptr;
static lv_obj_t *lbl_text = nullptr;

void refreshLVGL()
{
    lv_timer_handler();
}

void showStatus(const char *msg, bool fullRefresh = false)
{
    lv_label_set_text(lbl_status, msg);
    lv_label_set_text(lbl_text, "");
    lv_timer_handler();
    if (fullRefresh)
        inkplate.display();
    else
        inkplate.partialUpdate();
}

void showTextFile(const char *text)
{
    lv_label_set_text(lbl_status, "");
    lv_label_set_text(lbl_text, text);
    refreshLVGL();
    inkplate.display();
}

void createUI()
{
    lv_obj_clean(lv_screen_active());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    // Status label
    lbl_status = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_status, &lv_font_montserrat_24, 0);
    lv_label_set_long_mode(lbl_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_status, 540);
    lv_label_set_text(lbl_status, "");
    lv_obj_set_pos(lbl_status, 30, 30);

    // Main text label
    lbl_text = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_text, &lv_font_montserrat_18, 0);
    lv_label_set_long_mode(lbl_text, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_text, 560);
    lv_label_set_text(lbl_text, "");
    lv_obj_set_pos(lbl_text, 20, 20);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    createUI();

    // Clear display once at startup
    refreshLVGL();
    inkplate.display();

    inkplate.frontlight.setState(15);

    // Init SD card
    if (inkplate.sdCardInit())
    {
        showStatus("SD Card OK! Reading data...");
        delay(3000);

        // Try opening the file
        if (!file.open("/text.txt", O_RDONLY))
        {
            showStatus("File open error!", true);
        }
        else
        {
            char text[3001];
            int len = file.fileSize();

            if (len > 3000)
                len = 3000;

            file.read(text, len);
            text[len] = 0;

            showTextFile(text);

            inkplate.sdCardSleep();
        }
    }
    else
    {
        showStatus("SD Card error!", false);
        inkplate.sdCardSleep();

        while (true)
            ;
    }
}

void loop()
{
    // Nothing...
}