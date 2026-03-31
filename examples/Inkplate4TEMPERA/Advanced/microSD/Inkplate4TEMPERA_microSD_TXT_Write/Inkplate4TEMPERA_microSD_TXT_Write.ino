/**
 **************************************************
 * @file        Inkplate4TEMPERA_SD_TXT_Write_LVGL.ino
 * @brief       Write text file to SD card on Soldered Inkplate 4 TEMPERA using LVGL.
 *
 * @details     Demonstrates how to initialize the SD card, create a .txt file,
 *              write data into it, and display status messages using LVGL.
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

char *fileName = (char *)"test.txt";
char *dataToWrite = (char *)"Hello! This is the file writing example for Inkplate 4TEMPERA.\n";

// LVGL labels
static lv_obj_t *lbl_title = nullptr;
static lv_obj_t *lbl_status = nullptr;
static lv_obj_t *lbl_detail = nullptr;

void refreshLVGL()
{
    lv_tick_inc(20);
    lv_timer_handler();
}

void showMessage(const char *statusText, const char *detailText = "", bool fullRefresh = false)
{
    lv_label_set_text(lbl_status, statusText);
    lv_label_set_text(lbl_detail, detailText);
    refreshLVGL();

    if (fullRefresh)
        inkplate.display();
    else
        inkplate.partialUpdate();
}

void createUI()
{
    lv_obj_clean(lv_screen_active());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    lbl_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_24, 0);
    lv_label_set_text(lbl_title, "SD TXT Write Example");
    lv_obj_set_pos(lbl_title, 30, 30);

    lbl_status = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_status, &lv_font_montserrat_22, 0);
    lv_label_set_long_mode(lbl_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_status, 540);
    lv_label_set_text(lbl_status, "");
    lv_obj_set_pos(lbl_status, 30, 90);

    lbl_detail = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_detail, &lv_font_montserrat_18, 0);
    lv_label_set_long_mode(lbl_detail, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_detail, 540);
    lv_label_set_text(lbl_detail, "");
    lv_obj_set_pos(lbl_detail, 30, 150);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    createUI();

    // Initial clear screen
    refreshLVGL();
    inkplate.display();

    // Init SD card
    if (inkplate.sdCardInit())
    {
        showMessage("SD Card OK!");

        // Try to create/open test.txt
        if (!file.open(fileName, FILE_WRITE))
        {
            showMessage("SD Card OK!", "Error while creating the file!");
            inkplate.sdCardSleep();
        }
        else
        {
            showMessage("SD Card OK!", "Writing in the file...");
            file.write(dataToWrite);

            showMessage("SD Card OK!", "Data has been written!");
            file.close();
            inkplate.sdCardSleep();
        }
    }
    else
    {
        showMessage("", "SD Card error!");
        inkplate.sdCardSleep();

        while (true)
            ;
    }
}

void loop()
{
    // Nothing...
}