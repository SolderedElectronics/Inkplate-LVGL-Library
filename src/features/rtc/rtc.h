/**
 **************************************************
 * @file        rtc.h
 * @brief       Functionality for the onboard real time clock
 *
 *              https://github.com/e-radionicacom/Inkplate-Arduino-library
 *              For support, please reach over forums: forum.e-radionica.com/en
 *              For more info about the product, please check: www.inkplate.io
 *
 *              This code is released under the GNU Lesser General Public
 *              License v3.0: https://www.gnu.org/licenses/lgpl-3.0.en.html Please review the
 *              LICENSE file included with this example. If you have any questions about
 *              licensing, please contact techsupport@e-radionica.com Distributed as-is; no
 *              warranty is given.
 *
 * @authors     @ Soldered
 ***************************************************/
#ifndef _RTC_H_
#define _RTC_H_
#include "Arduino.h"
#include "Wire.h"

#define I2C_ADDR 0x51

// registar overview - crtl & status reg
#define RTC_CTRL_1 0x0
#define RTC_CTRL_2 0x01
#define RTC_OFFSET 0x02
#define RTC_RAM_by 0x03
// registar overview - time & data reg
#define RTC_SECOND_ADDR 0x04
#define RTC_MINUTE_ADDR 0x05
#define RTC_HOUR_ADDR   0x06
#define RTC_DAY_ADDR    0x07
#define RTC_WDAY_ADDR   0x08
#define RTC_MONTH_ADDR  0x09
// years 0-99; calculate real year = 2000 + RTC reg year
#define RTC_YEAR_ADDR 0x0A
// registar overview - alarm reg
#define RTC_SECOND_ALARM 0x0B
#define RTC_MINUTE_ALARM 0x0C
#define RTC_HOUR_ALARM   0x0D
#define RTC_DAY_ALARM    0x0E
#define RTC_WDAY_ALARM   0x0F
// registar overview - timer reg
#define RTC_TIMER_VAL   0x10
#define RTC_TIMER_MODE  0x11
#define RTC_TIMER_TCF   0x08
#define RTC_TIMER_TE    0x04
#define RTC_TIMER_TIE   0x02
#define RTC_TIMER_TI_TP 0x01
// format
// set AEN_x registers
#define RTC_ALARM 0x80
// set AIE ; enable/disable interrupt output pin
#define RTC_ALARM_AIE 0x80
// set AF register ; alarm flag needs to be cleared for alarm
#define RTC_ALARM_AF       0x40
#define RTC_CTRL_2_DEFAULT 0x00
#define RTC_TIMER_FLAG     0x08

#define RTC_ALARM_MATCH_SS      0b00000001
#define RTC_ALARM_MATCH_MMSS    0b00000011
#define RTC_ALARM_MATCH_HHMMSS  0b00000111
#define RTC_ALARM_MATCH_DHHMMSS 0b00001111
#define RTC_ALARM_MATCH_WHHMMSS 0b00010111

// Rtc internal capacitors
#define RTC_7PF    0
#define RTC_12_5PF 1

class RTC
{
  public:
    enum rtcCountdownSrcClock
    {
        TIMER_CLOCK_4096HZ = 0,
        TIMER_CLOCK_64HZ = 1,
        TIMER_CLOCK_1HZ = 2,
        TIMER_CLOCK_1PER60HZ = 3
    };

    void SetTime(uint8_t hour, uint8_t minute, uint8_t sec);
    void SetDate(uint8_t weekday, uint8_t day, uint8_t month, uint16_t yr);
    void SetEpoch(uint32_t _epoch);
    uint32_t GetEpoch();
    void GetRtcData();
    void EnableAlarm(); // called on setAlarm()
    void SetAlarm(uint8_t alarm_second, uint8_t alarm_minute, uint8_t alarm_hour, uint8_t alarm_day,
                  uint8_t alarm_weekday);
    void SetAlarmEpoch(uint32_t _epoch, uint8_t _match);
    void ReadAlarm();
    bool ChangeTimeFormat(); // returns false for 24H format & true for 12H format
    void TimerSet(rtcCountdownSrcClock source_clock, uint8_t value, bool int_enable, bool int_pulse);
    bool CheckTimerFlag();
    bool CheckAlarmFlag();
    void ClearAlarmFlag();
    void ClearTimerFlag();
    void DisableTimer();
    bool IsSet();
    void Reset();
    void SetInternalCapacitor(bool);
    void SetClockOffset(bool mode, int offsetValue);
    /* read RTC times */
    uint8_t GetSecond();
    uint8_t GetMinute();
    uint8_t GetHour();
    uint8_t GetDay();
    uint8_t GetWeekday();
    uint8_t GetMonth();
    uint16_t GetYear();
    /* read alarm times */ // if return = 99, no alarm
    uint8_t GetAlarmSecond();
    uint8_t GetAlarmMinute();
    uint8_t GetAlarmHour();
    uint8_t GetAlarmDay();
    uint8_t GetAlarmWeekday();


  private:
    uint8_t DecToBcd(uint8_t val);
    uint8_t BcdToDec(uint8_t val);
    /* alarm */
    uint8_t AlarmSecond;
    uint8_t AlarmMinute;
    uint8_t AlarmHour;
    uint8_t AlarmDay;
    uint8_t AlarmWeekday;
    /* support */
    uint8_t Control2;
    /* time variables*/
    uint8_t Hour;
    uint8_t Minute;
    uint8_t Second;
    uint8_t Day;
    uint8_t Weekday;
    uint8_t Month;
    uint16_t Year;
};
#endif