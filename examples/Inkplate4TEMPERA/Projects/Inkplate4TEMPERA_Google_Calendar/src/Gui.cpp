#include "Gui.h"
#include <ctime>
#include <Inkplate-LVGL.h>

Gui::Gui(Inkplate &inkplate) : inkplate(inkplate) {}

String Gui::getDayName(int dayIndex)
{
    const char *days[] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};
    return days[dayIndex];
}

String Gui::getMonthName(int monthIndex)
{
    const char *months[] = {"January", "February", "March", "April", "May", "June",
                            "July", "August", "September", "October", "November", "December"};
    return months[monthIndex];
}

String Gui::formatHour(const String &isoDateTime)
{
    if (isoDateTime.length() < 16)
        return "";
    return isoDateTime.substring(11, 16);
}

String Gui::formatDate(const String &isoDateTime)
{
    if (isoDateTime.length() < 10)
        return "";
    return isoDateTime.substring(8, 10);
}

String Gui::getShortDayName(int dayIndex)
{
    const char *shortDays[] = {"Sun", "Mon", "Tue", "Wed", "Thu", "Fri", "Sat"};
    return shortDays[dayIndex];
}

bool Gui::isCurrentEvent(const String &startTimeStr, const String &endTimeStr)
{
    struct tm timeInfo;
    if (!getLocalTime(&timeInfo))
        return false;

    time_t now = mktime(&timeInfo);

    struct tm startTm = {}, endTm = {};
    startTm.tm_year = startTimeStr.substring(0, 4).toInt() - 1900;
    startTm.tm_mon = startTimeStr.substring(5, 7).toInt() - 1;
    startTm.tm_mday = startTimeStr.substring(8, 10).toInt();
    startTm.tm_hour = startTimeStr.substring(11, 13).toInt();
    startTm.tm_min = startTimeStr.substring(14, 16).toInt();

    endTm.tm_year = endTimeStr.substring(0, 4).toInt() - 1900;
    endTm.tm_mon = endTimeStr.substring(5, 7).toInt() - 1;
    endTm.tm_mday = endTimeStr.substring(8, 10).toInt();
    endTm.tm_hour = endTimeStr.substring(11, 13).toInt();
    endTm.tm_min = endTimeStr.substring(14, 16).toInt();

    time_t start = mktime(&startTm);
    time_t end = mktime(&endTm);

    return (now >= start && now <= end);
}

static void guiRefresh(Inkplate &inkplate)
{
    lv_tick_inc(50);
    lv_timer_handler();
    inkplate.display();
}

static lv_obj_t *createText(lv_obj_t *parent,
                            const char *text,
                            int x,
                            int y,
                            const lv_font_t *font,
                            lv_color_t color,
                            int width = LV_SIZE_CONTENT,
                            lv_text_align_t align = LV_TEXT_ALIGN_LEFT)
{
    lv_obj_t *label = lv_label_create(parent);
    lv_label_set_text(label, text);
    lv_obj_set_pos(label, x, y);

    if (width != LV_SIZE_CONTENT)
    {
        lv_obj_set_width(label, width);
        lv_label_set_long_mode(label, LV_LABEL_LONG_CLIP);
    }

    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, color, 0);
    lv_obj_set_style_text_align(label, align, 0);
    return label;
}

static lv_obj_t *createBox(lv_obj_t *parent,
                           int x,
                           int y,
                           int w,
                           int h,
                           lv_color_t bgColor,
                           lv_opa_t bgOpa,
                           lv_color_t borderColor,
                           int borderWidth,
                           int radius = 0)
{
    lv_obj_t *obj = lv_obj_create(parent);
    lv_obj_set_pos(obj, x, y);
    lv_obj_set_size(obj, w, h);

    lv_obj_set_style_bg_color(obj, bgColor, 0);
    lv_obj_set_style_bg_opa(obj, bgOpa, 0);
    lv_obj_set_style_border_color(obj, borderColor, 0);
    lv_obj_set_style_border_width(obj, borderWidth, 0);
    lv_obj_set_style_radius(obj, radius, 0);
    lv_obj_set_style_shadow_width(obj, 0, 0);
    lv_obj_set_style_pad_all(obj, 0, 0);

    return obj;
}

static lv_obj_t *createBar(lv_obj_t *parent, int x, int y, int w, int h, lv_color_t color)
{
    lv_obj_t *bar = lv_obj_create(parent);
    lv_obj_set_pos(bar, x, y);
    lv_obj_set_size(bar, w, h);

    lv_obj_set_style_bg_color(bar, color, 0);
    lv_obj_set_style_bg_opa(bar, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(bar, 0, 0);
    lv_obj_set_style_radius(bar, 0, 0);
    lv_obj_set_style_shadow_width(bar, 0, 0);
    lv_obj_set_style_pad_all(bar, 0, 0);

    return bar;
}

void Gui::wifiError()
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    createText(screen, "WiFi connection failed.", 50, 140, &lv_font_montserrat_24, lv_color_hex(0x000000));
    createText(screen, "Check credentials or try again.", 50, 190, &lv_font_montserrat_20, lv_color_hex(0x000000));

    guiRefresh(inkplate);
}

void Gui::drawHeader(const String &title)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    createText(screen, title.c_str(), 10, 10, &lv_font_montserrat_28, lv_color_hex(0x000000));

    guiRefresh(inkplate);
}

void Gui::showCalendar(calendarData *calendar)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    createBox(screen, 0, 0, 600, 125, lv_color_hex(0x000000), LV_OPA_COVER, lv_color_hex(0x000000), 0, 0);

    struct tm timeInfo;
    if (!getLocalTime(&timeInfo))
    {
        showError("Time not available");
        return;
    }

    char timeString[6];
    sprintf(timeString, "%02d:%02d", timeInfo.tm_hour, timeInfo.tm_min);

    String dayNumber = String(timeInfo.tm_mday);
    String dayName = getDayName(timeInfo.tm_wday);
    String monthYear = getMonthName(timeInfo.tm_mon) + " " + String(1900 + timeInfo.tm_year);

    createText(screen, dayNumber.c_str(), 10, 18, &lv_font_montserrat_48, lv_color_hex(0xFFFFFF));
    createText(screen, dayName.c_str(), 125, 22, &lv_font_montserrat_28, lv_color_hex(0xFFFFFF));
    createText(screen, monthYear.c_str(), 125, 68, &lv_font_montserrat_24, lv_color_hex(0xFFFFFF));
    createText(screen, "Last Updated:", 430, 18, &lv_font_montserrat_14, lv_color_hex(0xFFFFFF));
    createText(screen, timeString, 500, 45, &lv_font_montserrat_20, lv_color_hex(0xFFFFFF));

    Event *events = calendar->getEvents();
    int eventCount = calendar->getEventCount();
    int y = 150;
    int x = 100;
    int xTime = 480;

    String lastDate = "";
    int counter = 0;

    for (int i = 0; i < eventCount; i++)
    {
        String eventDate = formatDate(events[i].startTime);

        if (eventDate != lastDate)
        {
            y += 35;

            struct tm timeStruct = {};
            timeStruct.tm_year = timeInfo.tm_year;
            timeStruct.tm_mon = timeInfo.tm_mon;
            timeStruct.tm_mday = eventDate.toInt();
            mktime(&timeStruct);

            createText(screen, eventDate.c_str(), 10, y - 22, &lv_font_montserrat_24, lv_color_hex(0x000000));
            createText(screen, getShortDayName(timeStruct.tm_wday).c_str(), 10, y + 10, &lv_font_montserrat_14, lv_color_hex(0x000000));

            lastDate = eventDate;
        }

        int yLineStart = y;

        bool isNow = isCurrentEvent(events[i].startTime, events[i].endTime);
        if (isNow)
        {
            createBox(screen, x - 10, y - 35, 490, 70,
                      lv_color_hex(0xD9D9D9), LV_OPA_COVER,
                      lv_color_hex(0xD9D9D9), 0, 10);
        }

        String summary = events[i].summary;
        if (summary.length() > MAX_SUMMARY_LENGTH)
        {
            summary = summary.substring(0, MAX_SUMMARY_LENGTH) + "...";
        }

        String startHour = formatHour(events[i].startTime);
        String endHour = formatHour(events[i].endTime);

        createText(screen, summary.c_str(), x, y - 22, &lv_font_montserrat_22, lv_color_hex(0x000000), 360, LV_TEXT_ALIGN_LEFT);
        createText(screen, startHour.c_str(), xTime, y - 22, &lv_font_montserrat_22, lv_color_hex(0x000000));
        createText(screen, endHour.c_str(), xTime + 25, y + 6, &lv_font_montserrat_14, lv_color_hex(0x666666));

        createBar(screen, 70, yLineStart - 43, 3, 68, lv_color_hex(0x000000));

        y += 50;
        counter = i;

        if (y >= 525)
        {
            break;
        }
    }

    if (counter == eventCount - 1 && y < 575)
    {
        createText(screen, "No more events in the next 2 weeks!", 100, y + 10, &lv_font_montserrat_14, lv_color_hex(0x666666));
    }

    guiRefresh(inkplate);
}

void Gui::showError(const String &message)
{
    lv_obj_t *screen = lv_screen_active();
    lv_obj_clean(screen);
    lv_obj_set_style_bg_color(screen, lv_color_hex(0xFFFFFF), LV_PART_MAIN);

    createText(screen, "Error:", 10, 10, &lv_font_montserrat_24, lv_color_hex(0x000000));
    createText(screen, message.c_str(), 10, 50, &lv_font_montserrat_20, lv_color_hex(0x000000), 560, LV_TEXT_ALIGN_LEFT);

    guiRefresh(inkplate);
}