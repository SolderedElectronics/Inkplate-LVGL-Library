/**
 **************************************************
 *
 * @file        RTCTimer.ino
 * @brief       RTC timer + live clock with partial updates (Inkplate 6 + LVGL)
 *
 *              Updates the on-screen time every second using fast partial refresh
 *              so it behaves like a clock. An RTC countdown timer is configured;
 *              when it fires, “Timer!” is appended to the label.
 *
 *
 * For info on how to quickly get started with Inkplate 6 visit:
 * https://soldered.com/documentation/inkplate/6/overview/
 *
 * @authors     Soldered
 * @date        November 2025
 ***************************************************
 */

#include <Inkplate-LVGL.h>

// -------- Settings --------
#define REFRESH_DELAY_MS      1000   // update screen every second
#define FULL_REFRESH_EVERY    60     // do a FULL refresh every 60 partials
static int countdown_time_sec = 60;  // RTC countdown seconds (one-shot)

// -------- Inkplate / LVGL --------
Inkplate inkplate(INKPLATE_1BIT);     // fast B/W mode

// -------- State --------
static unsigned long lastRefresh = 0;
static int partialCount = 0;
static lv_obj_t *label_time = nullptr;

// Forward declarations
static void updateTimeLabel();

void setup()
{
  // Init display in PARTIAL mode (fast clock-style updates)
  inkplate.begin(LV_DISP_RENDER_MODE_PARTIAL);

  // (Optional) Configure RTC INT pin (GPIO 39) as input pull-up if used externally
  pinMode(39, INPUT_PULLUP);

  // If needed, set initial date/time (example values)
  // inkplate.rtc.reset(); // uncomment to clear RTC for demo
  if (!inkplate.rtc.isSet()) {
    inkplate.rtc.setTime(14, 30, 0);       // 14:30:00
    inkplate.rtc.setDate(3, 12, 11, 2025); // Wednesday, 12.11.2025 (weekday=3)
  }

  // Configure RTC timer: 1 Hz source, N seconds, interrupt enabled, not periodic
  // (Set 'true, true' for periodic behavior if you want it to repeat.)
  inkplate.rtc.timerSet(RTC::TIMER_CLOCK_1HZ, countdown_time_sec, true, false);

  // White background
  lv_obj_set_style_bg_color(lv_screen_active(), lv_color_hex(0xFFFFFF), LV_PART_MAIN);

  // Centered label (default font to avoid config issues)
  label_time = lv_label_create(lv_screen_active());
  lv_obj_set_style_text_color(label_time, lv_color_hex(0x000000), LV_PART_MAIN);
  lv_obj_set_style_text_font(label_time, LV_FONT_DEFAULT, 0);
  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);

  // Initial draw
  updateTimeLabel();
  lv_tick_inc(20);
  lv_timer_handler();
  inkplate.display(); // initial FULL to start clean

  lastRefresh = millis();
  partialCount = 0;
}

void loop()
{
  if (millis() - lastRefresh >= REFRESH_DELAY_MS) {
    updateTimeLabel();

    lv_tick_inc(10);
    lv_timer_handler();

    // Do fast partials most of the time; occasional FULL to reduce ghosting
    if (++partialCount >= FULL_REFRESH_EVERY) {
      inkplate.display();      // FULL
      partialCount = 0;
    } else {
      inkplate.partialUpdate(); // PARTIAL
    }

    lastRefresh = millis();
  }
}

// ----- Helpers -----

// Update label with current time/date and show "Timer!" if RTC timer fired
static void updateTimeLabel()
{
  inkplate.rtc.getRtcData();

  uint8_t hour   = inkplate.rtc.getHour();
  uint8_t minute = inkplate.rtc.getMinute();
  uint8_t second = inkplate.rtc.getSecond();
  uint8_t day    = inkplate.rtc.getDay();
  uint8_t month  = inkplate.rtc.getMonth();
  uint16_t year  = inkplate.rtc.getYear();
  uint8_t weekday = inkplate.rtc.getWeekday();

  const char *wdayNames[] = {
    "Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"
  };

  char buf[160];
  snprintf(buf, sizeof(buf),
           "%02d:%02d:%02d\n%s, %02d/%02d/%04d",
           hour, minute, second, wdayNames[weekday], day, month, year);

  // If the RTC timer event occurred, show a "Timer!" message
  if (inkplate.rtc.checkTimerFlag()) {
    inkplate.rtc.clearTimerFlag(); // clear the flag
    // inkplate.rtc.disableTimer(); // uncomment for one-shot; leave enabled for repeat (if periodic)
    strlcat(buf, "\n\nTimer!", sizeof(buf));
  }

  lv_label_set_text(label_time, buf);
  lv_obj_align(label_time, LV_ALIGN_CENTER, 0, 0);
}
