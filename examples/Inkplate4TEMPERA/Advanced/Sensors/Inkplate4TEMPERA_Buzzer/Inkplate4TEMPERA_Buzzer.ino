/**
 **************************************************
 * @file        Inkplate4TEMPERA_Buzzer_LVGL.ino
 * @brief       Demonstrates how to drive the built-in buzzer on Inkplate 4
 *              TEMPERA, including simple beeps and approximate pitch control.
 *
 * @details     LVGL rewrite of the original example.
 *              The buzzer behavior is kept the same, while LVGL labels are used
 *              to show the currently playing demo or note on the screen.
 *
 * @author      Soldered
 * @date        2026
 **************************************************/
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>

Inkplate inkplate(INKPLATE_1BIT);

// Frequencies for the C Maj7 chord (in Hz):
// C, E, G, B
int chord[4] = {523, 659, 783, 987};
const char *noteNames[4] = {"C", "E", "G", "B"};

// Helpful variables for playing the chord
int currentNoteIndex = 0;
int repeatCounter = 0;

// LVGL objects
static lv_obj_t *lbl_title = nullptr;
static lv_obj_t *lbl_status = nullptr;
static lv_obj_t *lbl_note = nullptr;
static lv_obj_t *lbl_info = nullptr;

void refreshLVGL()
{
    lv_timer_handler();
}

void showStatus(const char *statusText, const char *noteText = "")
{
    lv_label_set_text(lbl_status, statusText);
    lv_label_set_text(lbl_note, noteText);
    refreshLVGL();
    inkplate.display();
}

void createUI()
{
    lv_obj_clean(lv_screen_active());

    lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);
    lv_obj_set_style_bg_opa(lv_screen_active(), LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_text_color(lv_screen_active(), lv_color_hex(0x000000), LV_PART_MAIN);

    lbl_title = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_title, &lv_font_montserrat_24, 0);
    lv_label_set_text(lbl_title, "Inkplate 4 TEMPERA Buzzer Demo");
    lv_obj_align(lbl_title, LV_ALIGN_TOP_MID, 0, 30);

    lbl_status = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_status, &lv_font_montserrat_22, 0);
    lv_label_set_long_mode(lbl_status, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_status, 520);
    lv_label_set_text(lbl_status, "Initializing...");
    lv_obj_align(lbl_status, LV_ALIGN_TOP_LEFT, 40, 130);

    lbl_note = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_note, &lv_font_montserrat_32, 0);
    lv_label_set_text(lbl_note, "");
    lv_obj_align(lbl_note, LV_ALIGN_TOP_LEFT, 40, 240);

    lbl_info = lv_label_create(lv_screen_active());
    lv_obj_set_style_text_font(lbl_info, &lv_font_montserrat_18, 0);
    lv_label_set_long_mode(lbl_info, LV_LABEL_LONG_WRAP);
    lv_obj_set_width(lbl_info, 520);
    lv_label_set_text(lbl_info,
                      "Startup demo: short beeps, manual on/off beeps, "
                      "low/high pitch demo, then repeating chord melody.");
    lv_obj_align(lbl_info, LV_ALIGN_BOTTOM_LEFT, 40, -40);
}

void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);

    createUI();
    refreshLVGL();
    inkplate.display();

    // Init the buzzer
    inkplate.buzzer.init();

    // Demo 1: three short beeps
    showStatus("Demo 1: three short beeps");
    inkplate.buzzer.beep(80);
    delay(80);
    inkplate.buzzer.beep(80);
    delay(80);
    inkplate.buzzer.beep(80);
    delay(80);

    delay(5000);

    // Demo 2: manual on/off control
    showStatus("Demo 2: manual on/off beeps");
    inkplate.buzzer.beepOn();
    delay(200);
    inkplate.buzzer.beepOff();
    delay(200);
    inkplate.buzzer.beepOn();
    delay(200);
    inkplate.buzzer.beepOff();
    delay(200);

    delay(5000);

    // Demo 3: low/high pitch control
    showStatus("Demo 3: low and high frequency beeps");
    inkplate.buzzer.beep(300, 750);
    delay(50);
    inkplate.buzzer.beep(300, 750);
    delay(50);
    inkplate.buzzer.beep(300, 2400);
    delay(50);
    inkplate.buzzer.beep(300, 2400);
    delay(50);

    delay(5000);

    showStatus("Loop demo: C Maj7 melody", "Now playing...");
}

void loop()
{
    char noteBuf[64];

    // First two cycles: play note once
    if (repeatCounter < 2)
    {
        snprintf(noteBuf, sizeof(noteBuf), "Note: %s (%d Hz)", noteNames[currentNoteIndex], chord[currentNoteIndex]);
        showStatus("Loop demo: single notes", noteBuf);

        inkplate.buzzer.beep(100, chord[currentNoteIndex]);
        delay(600);
    }
    // Next two cycles: play note twice
    else
    {
        snprintf(noteBuf, sizeof(noteBuf), "Note: %s (%d Hz) x2", noteNames[currentNoteIndex], chord[currentNoteIndex]);
        showStatus("Loop demo: doubled notes", noteBuf);

        inkplate.buzzer.beep(100, chord[currentNoteIndex]);
        delay(250);
        inkplate.buzzer.beep(50, chord[currentNoteIndex]);
        delay(300);
    }

    currentNoteIndex++;

    if (currentNoteIndex >= 4)
    {
        currentNoteIndex = 0;
        repeatCounter++;

        if (repeatCounter >= 4)
        {
            repeatCounter = 0;
            showStatus("Loop demo complete, pausing...", "");
            delay(3000);
            showStatus("Loop demo: C Maj7 melody", "Restarting...");
        }
    }
}