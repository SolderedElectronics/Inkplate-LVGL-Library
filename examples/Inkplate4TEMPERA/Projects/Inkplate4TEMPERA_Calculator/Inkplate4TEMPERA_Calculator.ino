/**
 **************************************************
 * @file        Inkplate4TEMPERA_Calculator_LVGL.ino
 * @brief       LVGL touchscreen calculator for Inkplate 4 TEMPERA
 *
 * @details     Port of the classic calculator example to the Inkplate-LVGL
 *              framework. Uses LVGL buttons, labels and event callbacks.
 *
 **************************************************/
#ifndef ARDUINO_INKPLATE4TEMPERA
#error "Wrong board selection for this example, please select Inkplate 4 TEMPERA in the boards menu."
#endif

#include <Inkplate-LVGL.h>
#include <stdlib.h>

Inkplate inkplate(INKPLATE_1BIT);


String leftInput = "";
String rightInput = "";
char currentOp = '\0';
String currentExpression = "";
String historyLine = "";

bool decimalPointOnCurrentNumber = false;
int numOfDecimalDigitsOnCurrentNumber = 0;
int numOfDigitsEntered = 0;

int partialRefreshCount = 0;
#define NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH 12


static lv_obj_t *lblHistory = nullptr;
static lv_obj_t *lblExpression = nullptr;
static lv_obj_t *lblStatus = nullptr;


void createUI();
void createButton(lv_obj_t *parent, const char *txt, int x, int y, int w, int h, lv_event_cb_t cb);
void refreshDisplay(bool forceFull = false);
void resetCurrentEntryTracking();
void clearInputOnly();
void clearHistoryOnly();
void updateDisplayText();
void appendDigit(char digit);
void appendDecimalPoint();
void appendOperator(char op);
void performCalculation();
double calculate(double leftNumber, double rightNumber, char op);
void onDigitButton(lv_event_t *e);
void onOperatorButton(lv_event_t *e);
void onEqualsButton(lv_event_t *e);
void onDecimalButton(lv_event_t *e);
void onClearInputButton(lv_event_t *e);
void onClearHistoryButton(lv_event_t *e);
void onRefreshButton(lv_event_t *e);


void refreshDisplay(bool forceFull)
{
    if (forceFull || partialRefreshCount >= NUM_PARTIAL_UPDATES_BEFORE_FULL_REFRESH)
    {
        lv_timer_handler();
        inkplate.display();
        partialRefreshCount = 0;
    }
    else
    {
        lv_timer_handler();
        inkplate.partialUpdate(false, true);
        partialRefreshCount++;
    }
}

void resetCurrentEntryTracking()
{
    decimalPointOnCurrentNumber = false;
    numOfDecimalDigitsOnCurrentNumber = 0;
    numOfDigitsEntered = 0;
}

void clearInputOnly()
{
    leftInput = "";
    rightInput = "";
    currentOp = '\0';
    currentExpression = "";
    resetCurrentEntryTracking();
    lv_label_set_text(lblStatus, "");
    updateDisplayText();
}

void clearHistoryOnly()
{
    historyLine = "";
    lv_label_set_text(lblStatus, "");
    updateDisplayText();
}

void updateDisplayText()
{
    lv_label_set_text(lblHistory, historyLine.c_str());

    if (currentExpression.length() == 0)
    {
        lv_label_set_text(lblExpression, "");
    }
    else
    {
        lv_label_set_text(lblExpression, currentExpression.c_str());
    }
}

double calculate(double leftNumber, double rightNumber, char op)
{
    switch (op)
    {
        case '+': return leftNumber + rightNumber;
        case '-': return leftNumber - rightNumber;
        case 'x': return leftNumber * rightNumber;
        case '/': return leftNumber / rightNumber;
        default:  return 0.0;
    }
}

void appendDigit(char digit)
{
    if (numOfDigitsEntered >= 6) return;
    if (decimalPointOnCurrentNumber && numOfDecimalDigitsOnCurrentNumber >= 2) return;

    if (currentOp == '\0')
    {
        leftInput += digit;
    }
    else
    {
        rightInput += digit;
    }

    currentExpression += digit;
    numOfDigitsEntered++;

    if (decimalPointOnCurrentNumber)
    {
        numOfDecimalDigitsOnCurrentNumber++;
    }

    lv_label_set_text(lblStatus, "");
    updateDisplayText();
    refreshDisplay();
}

void appendDecimalPoint()
{
    if (decimalPointOnCurrentNumber) return;
    if (numOfDigitsEntered >= 6) return;

    if (currentOp == '\0')
    {
        if (leftInput.length() == 0)
        {
            leftInput = "0";
            currentExpression = "0";
            numOfDigitsEntered++;
        }
        leftInput += ".";
    }
    else
    {
        if (rightInput.length() == 0)
        {
            rightInput = "0";
            currentExpression += "0";
            numOfDigitsEntered++;
        }
        rightInput += ".";
    }

    currentExpression += ".";
    decimalPointOnCurrentNumber = true;

    lv_label_set_text(lblStatus, "");
    updateDisplayText();
    refreshDisplay();
}

void appendOperator(char op)
{
    if (currentOp != '\0') return;
    if (leftInput.length() == 0) return;

    currentOp = op;
    currentExpression += " ";
    currentExpression += op;
    currentExpression += " ";

    resetCurrentEntryTracking();

    lv_label_set_text(lblStatus, "");
    updateDisplayText();
    refreshDisplay();
}

void performCalculation()
{
    if (currentOp == '\0') return;
    if (leftInput.length() == 0 || rightInput.length() == 0) return;

    double leftNumber = atof(leftInput.c_str());
    double rightNumber = atof(rightInput.c_str());

    if (currentOp == '/' && rightNumber == 0.0)
    {
        lv_label_set_text(lblStatus, "Division by zero is not allowed");
        refreshDisplay();
        return;
    }

    double result = calculate(leftNumber, rightNumber, currentOp);

    String resultString = String(result, 4);

    // Trim trailing zeroes
    while (resultString.endsWith("0"))
    {
        resultString.remove(resultString.length() - 1);
    }
    if (resultString.endsWith("."))
    {
        resultString.remove(resultString.length() - 1);
    }

    historyLine = currentExpression + " = " + resultString;

    leftInput = "";
    rightInput = "";
    currentOp = '\0';
    currentExpression = "";
    resetCurrentEntryTracking();

    lv_label_set_text(lblStatus, "");
    updateDisplayText();
    refreshDisplay();
}


void onDigitButton(lv_event_t *e)
{
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    const char *txt = lv_label_get_text(label);

    if (txt && txt[0] >= '0' && txt[0] <= '9')
    {
        appendDigit(txt[0]);
    }
}

void onOperatorButton(lv_event_t *e)
{
    lv_obj_t *btn = (lv_obj_t *)lv_event_get_target(e);
    lv_obj_t *label = lv_obj_get_child(btn, 0);
    const char *txt = lv_label_get_text(label);

    if (!txt) return;

    if (txt[0] == '+') appendOperator('+');
    else if (txt[0] == '-') appendOperator('-');
    else if (txt[0] == 'x' || txt[0] == 'X') appendOperator('x');
    else if (txt[0] == '/') appendOperator('/');
}

void onEqualsButton(lv_event_t *e)
{
    (void)e;
    performCalculation();
}

void onDecimalButton(lv_event_t *e)
{
    (void)e;
    appendDecimalPoint();
}

void onClearInputButton(lv_event_t *e)
{
    (void)e;
    clearInputOnly();
    refreshDisplay();
}

void onClearHistoryButton(lv_event_t *e)
{
    (void)e;
    clearHistoryOnly();
    refreshDisplay();
}

void onRefreshButton(lv_event_t *e)
{
    (void)e;
    lv_label_set_text(lblStatus, "");
    updateDisplayText();
    refreshDisplay(true);
}


void createButton(lv_obj_t *parent, const char *txt, int x, int y, int w, int h, lv_event_cb_t cb)
{
    lv_obj_t *btn = lv_btn_create(parent);
    lv_obj_set_size(btn, w, h);
    lv_obj_set_pos(btn, x, y);

    lv_obj_set_style_radius(btn, 0, 0);
    lv_obj_set_style_bg_color(btn, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(btn, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(btn, lv_color_black(), 0);
    lv_obj_set_style_border_width(btn, 2, 0);
    lv_obj_set_style_shadow_width(btn, 0, 0);

    lv_obj_add_event_cb(btn, cb, LV_EVENT_CLICKED, NULL);

    lv_obj_t *label = lv_label_create(btn);
    lv_label_set_text(label, txt);
    lv_obj_set_style_text_color(label, lv_color_black(), 0);
    lv_obj_set_style_text_font(label, &lv_font_montserrat_32, 0);
    lv_obj_center(label);
}

void createUI()
{
    lv_obj_t *screen = lv_scr_act();
    lv_obj_clean(screen);

    lv_obj_set_style_bg_color(screen, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(screen, LV_OPA_COVER, 0);

    // Result/history area
    lv_obj_t *historyBox = lv_obj_create(screen);
    lv_obj_set_size(historyBox, 370, 75);
    lv_obj_set_pos(historyBox, 200, 30);
    lv_obj_set_style_radius(historyBox, 0, 0);
    lv_obj_set_style_bg_color(historyBox, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(historyBox, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(historyBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(historyBox, 2, 0);
    lv_obj_set_style_shadow_width(historyBox, 0, 0);
    lv_obj_set_style_pad_all(historyBox, 8, 0);

    lblHistory = lv_label_create(historyBox);
    lv_label_set_text(lblHistory, "");
    lv_obj_set_style_text_color(lblHistory, lv_color_black(), 0);
    lv_obj_set_style_text_font(lblHistory, &lv_font_montserrat_16, 0);
    lv_obj_align(lblHistory, LV_ALIGN_LEFT_MID, 0, 0);

    // Expression area
    lv_obj_t *exprBox = lv_obj_create(screen);
    lv_obj_set_size(exprBox, 370, 75);
    lv_obj_set_pos(exprBox, 200, 105);
    lv_obj_set_style_radius(exprBox, 0, 0);
    lv_obj_set_style_bg_color(exprBox, lv_color_white(), 0);
    lv_obj_set_style_bg_opa(exprBox, LV_OPA_COVER, 0);
    lv_obj_set_style_border_color(exprBox, lv_color_black(), 0);
    lv_obj_set_style_border_width(exprBox, 2, 0);
    lv_obj_set_style_shadow_width(exprBox, 0, 0);
    lv_obj_set_style_pad_all(exprBox, 8, 0);

    lblExpression = lv_label_create(exprBox);
    lv_label_set_text(lblExpression, "");
    lv_obj_set_width(lblExpression, 350);
    lv_label_set_long_mode(lblExpression, LV_LABEL_LONG_CLIP);
    lv_obj_set_style_text_align(lblExpression, LV_TEXT_ALIGN_RIGHT, 0);
    lv_obj_set_style_text_color(lblExpression, lv_color_black(), 0);
    lv_obj_set_style_text_font(lblExpression, &lv_font_montserrat_22, 0);
    lv_obj_align(lblExpression, LV_ALIGN_RIGHT_MID, 0, 0);

    // Left-side controls
    createButton(screen, "Clear result", 30, 30, 170, 50, onClearHistoryButton);
    createButton(screen, "Clear Input", 30, 80, 170, 50, onClearInputButton);
    createButton(screen, "Refresh", 30, 130, 170, 50, onRefreshButton);

    // Keypad buttons
    createButton(screen, "7",  30, 180, 135, 97, onDigitButton);
    createButton(screen, "8", 165, 180, 135, 97, onDigitButton);
    createButton(screen, "9", 300, 180, 135, 97, onDigitButton);
    createButton(screen, "/", 435, 180, 135, 97, onOperatorButton);

    createButton(screen, "4",  30, 277, 135, 97, onDigitButton);
    createButton(screen, "5", 165, 277, 135, 97, onDigitButton);
    createButton(screen, "6", 300, 277, 135, 97, onDigitButton);
    createButton(screen, "x", 435, 277, 135, 97, onOperatorButton);

    createButton(screen, "1",  30, 374, 135, 97, onDigitButton);
    createButton(screen, "2", 165, 374, 135, 97, onDigitButton);
    createButton(screen, "3", 300, 374, 135, 97, onDigitButton);
    createButton(screen, "-", 435, 374, 135, 97, onOperatorButton);

    createButton(screen, ".",  30, 471, 135, 97, onDecimalButton);
    createButton(screen, "0", 165, 471, 135, 97, onDigitButton);
    createButton(screen, "=", 300, 471, 135, 97, onEqualsButton);
    createButton(screen, "+", 435, 471, 135, 97, onOperatorButton);

    // Status line
    lblStatus = lv_label_create(screen);
    lv_label_set_text(lblStatus, "");
    lv_obj_set_style_text_color(lblStatus, lv_color_black(), 0);
    lv_obj_set_style_text_font(lblStatus, &lv_font_montserrat_16, 0);
    lv_obj_set_pos(lblStatus, 30, 575);

    updateDisplayText();
}


void setup()
{
    Serial.begin(115200);
    delay(200);

    inkplate.begin(LV_DISPLAY_RENDER_MODE_PARTIAL);
    inkplate.selectDisplayMode(INKPLATE_1BIT);
    inkplate.setRotation(0);

    if (inkplate.touchscreen.init(true))
    {
        Serial.println("Touchscreen initialized.");
    }
    else
    {
        Serial.println("Touchscreen initialization failed.");
        while (true);
    }

    createUI();

    lv_timer_handler();
    inkplate.display();
}

void loop()
{
    lv_timer_handler();
    delay(20);
}