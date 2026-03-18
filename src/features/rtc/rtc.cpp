/**
 **************************************************
 * @file        rtc.cpp
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

#include "rtc.h"


/**
 * @brief                   Method to set time
 *
 * @param uint8_t rtcHour      Set the rtcHour
 * @param uint8_t rtcMinute    Set the minutes
 * @param uint8_t rtcSecond    Set the seconds
 */
void RTC::SetTime(uint8_t rtcHour, uint8_t rtcMinute, uint8_t rtcSecond)
{
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_RAM_by);
    Wire.write(170); // Write in RAM 170 to know that RTC is set
    Wire.write(DecToBcd(rtcSecond));
    Wire.write(DecToBcd(rtcMinute));
    Wire.write(DecToBcd(rtcHour));
    Wire.endTransmission();
}

/**
 * @brief                   Method to set the date
 *
 * @param uint8_t rtcWeekday   Set the rtcWeekday
 * @param uint8_t rtcDay       Set the rtcDay
 * @param uint8_t rtcMonth     Set the rtcMonth
 * @param uint8_t yr        Set the rtcYear
 */
void RTC::SetDate(uint8_t rtcWeekday, uint8_t rtcDay, uint8_t rtcMonth, uint16_t yr)
{
    Year = yr - 2000; // convert to RTC rtcYear format 0-99

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_RAM_by);
    Wire.write(170); // Write in RAM 170 to know that RTC is set
    Wire.endTransmission();

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_DAY_ADDR);
    Wire.write(DecToBcd(rtcDay));
    Wire.write(DecToBcd(rtcWeekday));
    Wire.write(DecToBcd(rtcMonth));
    Wire.write(DecToBcd(Year));
    Wire.endTransmission();
}

/**
 * @brief                   Method to set time and date using epoch
 *
 * @param uint32_t _epoch   Set RTC epoch
 */
void RTC::SetEpoch(uint32_t _epoch)
{
    struct tm _t;
    time_t _e = _epoch;
    memcpy(&_t, localtime((const time_t *)&_e), sizeof(_t));

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_RAM_by);
    Wire.write(170);
    Wire.write(DecToBcd(_t.tm_sec));
    Wire.write(DecToBcd(_t.tm_min));
    Wire.write(DecToBcd(_t.tm_hour));
    Wire.write(DecToBcd(_t.tm_mday));
    Wire.write(DecToBcd(_t.tm_wday));
    Wire.write(DecToBcd(_t.tm_mon + 1));
    Wire.write(DecToBcd(_t.tm_year + 1900 - 2000));
    Wire.endTransmission();
}

/**
 * @brief                   Method to get time and date using epoch
 *
 * @returns uint32_t        Returns the current epoch
 */
uint32_t RTC::GetEpoch()
{
    struct tm _t;

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_SECOND_ADDR);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 7); // ignore bit 7
    _t.tm_sec = BcdToDec(Wire.read() & 0x7F);
    _t.tm_min = BcdToDec(Wire.read() & 0x7F);
    _t.tm_hour = BcdToDec(Wire.read() & 0x3F);
    _t.tm_mday = BcdToDec(Wire.read() & 0x3F);
    _t.tm_wday = BcdToDec(Wire.read() & 0x07);
    _t.tm_mon = BcdToDec(Wire.read() & 0x1F) - 1;
    _t.tm_year = BcdToDec(Wire.read()) + 2000 - 1900;
    Wire.endTransmission();

    return (uint32_t)(mktime(&_t));
}

/**
 * @brief                   Reads time and date from the RTC
 */
void RTC::GetRtcData()
{
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_SECOND_ADDR); // datasheet 8.4.
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 7);

    while (Wire.available())
    {
        Second = BcdToDec(Wire.read() & 0x7F); // ignore bit 7
        Minute = BcdToDec(Wire.read() & 0x7F);
        Hour = BcdToDec(Wire.read() & 0x3F); // ignore bits 7 & 6
        Day = BcdToDec(Wire.read() & 0x3F);
        Weekday = BcdToDec(Wire.read() & 0x07); // ignore bits 7,6,5,4 & 3
        Month = BcdToDec(Wire.read() & 0x1F);   // ignore bits 7,6 & 5
        Year = BcdToDec(Wire.read()) + 2000;
    }
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current seconds
 */
uint8_t RTC::GetSecond()
{
    return Second;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current minutes
 */
uint8_t RTC::GetMinute()
{
    return Minute;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current hours
 */
uint8_t RTC::GetHour()
{
    return Hour;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current rtcDay
 */
uint8_t RTC::GetDay()
{
    return Day;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current rtcWeekday
 */
uint8_t RTC::GetWeekday()
{
    return Weekday;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current rtcMonth
 */
uint8_t RTC::GetMonth()
{
    return Month;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current rtcYear
 */
uint16_t RTC::GetYear()
{
    return Year;
}

/**
 * @brief                   Enables the alarm of the RTC
 */
void RTC::EnableAlarm() // datasheet 8.5.6.
{
    // check Table 2. Control_2
    Control2 = RTC_CTRL_2_DEFAULT | RTC_ALARM_AIE; // enable interrupt
    Control2 &= ~RTC_ALARM_AF;                     // clear alarm flag

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(Control2);
    Wire.endTransmission();
}

/**
 * @brief                       Sets the alarm to all the params
 *
 * @param uint8_t AlarmSecond  Set the alarm seconds
 * @param uint8_t AlarmMinute  Set the alarm minutes
 * @param uint8_t AlarmHour    Set the alarm hours
 * @param uint8_t AlarmDay     Set the alarm rtcDay
 * @param uint8_t AlarmWeekday Set the alarm rtcWeekday
 */
void RTC::SetAlarm(uint8_t AlarmSecond, uint8_t AlarmMinute, uint8_t AlarmHour, uint8_t AlarmDay, uint8_t AlarmWeekday)
{
    if (AlarmSecond < 99)
    { // rtcSecond
        AlarmSecond = constrain(AlarmSecond, 0, 59);
        AlarmSecond = DecToBcd(AlarmSecond);
        AlarmSecond &= ~RTC_ALARM;
    }
    else
    {
        AlarmSecond = 0x0;
        AlarmSecond |= RTC_ALARM;
    }

    if (AlarmMinute < 99)
    { // rtcMinute
        AlarmMinute = constrain(AlarmMinute, 0, 59);
        AlarmMinute = DecToBcd(AlarmMinute);
        AlarmMinute &= ~RTC_ALARM;
    }
    else
    {
        AlarmMinute = 0x0;
        AlarmMinute |= RTC_ALARM;
    }

    if (AlarmHour < 99)
    { // rtcHour
        AlarmHour = constrain(AlarmHour, 0, 23);
        AlarmHour = DecToBcd(AlarmHour);
        AlarmHour &= ~RTC_ALARM;
    }
    else
    {
        AlarmHour = 0x0;
        AlarmHour |= RTC_ALARM;
    }

    if (AlarmDay < 99)
    { // rtcDay
        AlarmDay = constrain(AlarmDay, 1, 31);
        AlarmDay = DecToBcd(AlarmDay);
        AlarmDay &= ~RTC_ALARM;
    }
    else
    {
        AlarmDay = 0x0;
        AlarmDay |= RTC_ALARM;
    }

    if (AlarmWeekday < 99)
    { // rtcWeekday
        AlarmWeekday = constrain(AlarmWeekday, 0, 6);
        AlarmWeekday = DecToBcd(AlarmWeekday);
        AlarmWeekday &= ~RTC_ALARM;
    }
    else
    {
        AlarmWeekday = 0x0;
        AlarmWeekday |= RTC_ALARM;
    }

    EnableAlarm();

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_SECOND_ALARM);
    Wire.write(AlarmSecond);
    Wire.write(AlarmMinute);
    Wire.write(AlarmHour);
    Wire.write(AlarmDay);
    Wire.write(AlarmWeekday);
    Wire.endTransmission();
}

/**
 * @brief                   Set alarm using epoch
 *
 * @param uint32_t _epoch   RTC Epoch alarm
 * @param uint8_t _match    RTC Match
 */
void RTC::SetAlarmEpoch(uint32_t _epoch, uint8_t _match)
{
    struct tm _t;
    time_t _e = _epoch;

    memcpy(&_t, localtime((const time_t *)&_e), sizeof(_t));

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_SECOND_ALARM);
    Wire.write(DecToBcd(_t.tm_sec) & (~((_match & 1) << 7)));
    Wire.write(DecToBcd(_t.tm_min) & (~(((_match >> 1) & 1) << 7)));
    Wire.write(DecToBcd(_t.tm_hour) & (~(((_match >> 2) & 1) << 7)));
    Wire.write(DecToBcd(_t.tm_mday) & (~(((_match >> 3) & 1) << 7)));
    Wire.write(DecToBcd(_t.tm_wday) & (~(((_match >> 4) & 1) << 7)));
    Wire.endTransmission();

    EnableAlarm();
}


/**
 * @brief                   Reads the alarm of the RTC
 */
void RTC::ReadAlarm()
{
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_SECOND_ALARM); // datasheet 8.4.
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 5);

    while (Wire.available())
    {
        AlarmSecond = Wire.read();   // read RTC_SECOND_ALARM register
        if (RTC_ALARM & AlarmSecond) // check is AEN = 1 (rtcSecond alarm disabled)
        {
            AlarmSecond = 99; // using 99 as code for no alarm
        }
        else
        {                                                     // else if AEN = 0 (rtcSecond alarm enabled)
            AlarmSecond = BcdToDec(AlarmSecond & ~RTC_ALARM); // remove AEN flag and convert to dec number
        }

        AlarmMinute = Wire.read(); // rtcMinute
        if (RTC_ALARM & AlarmMinute)
        {
            AlarmMinute = 99;
        }
        else
        {
            AlarmMinute = BcdToDec(AlarmMinute & ~RTC_ALARM);
        }

        AlarmHour = Wire.read(); // rtcHour
        if (RTC_ALARM & AlarmHour)
        {
            AlarmHour = 99;
        }
        else
        {
            AlarmHour = BcdToDec(AlarmHour & 0x3F); // remove bits 7 & 6
        }

        AlarmDay = Wire.read(); // rtcDay
        if (RTC_ALARM & AlarmDay)
        {
            AlarmDay = 99;
        }
        else
        {
            AlarmDay = BcdToDec(AlarmDay & 0x3F); // remove bits 7 & 6
        }

        AlarmWeekday = Wire.read(); // rtcWeekday
        if (RTC_ALARM & AlarmWeekday)
        {
            AlarmWeekday = 99;
        }
        else
        {
            AlarmWeekday = BcdToDec(AlarmWeekday & 0x07); // remove bits 7,6,5,4 & 3
        }
    }
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current alarm seconds
 */
uint8_t RTC::GetAlarmSecond()
{
    ReadAlarm();
    return AlarmSecond;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current alarm minutes
 */
uint8_t RTC::GetAlarmMinute()
{
    ReadAlarm();
    return AlarmMinute;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current alarm hours
 */
uint8_t RTC::GetAlarmHour()
{
    ReadAlarm();
    return AlarmHour;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current alarm rtcDay
 */
uint8_t RTC::GetAlarmDay()
{
    ReadAlarm();
    return AlarmDay;
}

/**
 * @brief                   Small user method
 *
 * @returns uint8_t         Returns the current alarm rtcWeekday
 */
uint8_t RTC::GetAlarmWeekday()
{
    ReadAlarm();
    return AlarmWeekday;
}

/**
 * @brief                   Sets the timer countdown
 *
 * @param                   rtcCountdownSrcClock source_clock
 *                          timer clock frequency
 *
 * @param                   timer clock frequency
 *                          value to write in timer register
 *
 * @param                   bool int_enable
 *                          timer interrupt enable, 0 means no interrupt generated from timer
 *                          , 1 means interrupt is generated from timer
 *
 * @param                   bool int_pulse
 *                          timer interrupt mode, 0 means interrupt follows timer flag
 *                          , 1 means interrupt generates a pulse
 */
void RTC::TimerSet(rtcCountdownSrcClock source_clock, uint8_t value, bool int_enable, bool int_pulse)
{
    uint8_t timer_reg[2] = {0};

    // disable the countdown timer
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_TIMER_MODE);
    Wire.write(0x18); // default
    Wire.endTransmission();

    // clear Control_2
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(0x00); // default
    Wire.endTransmission();

    // reconfigure timer
    timer_reg[1] |= RTC_TIMER_TE; // enable timer
    if (int_enable)
        timer_reg[1] |= RTC_TIMER_TIE; // enable interrupt
    if (int_pulse)
        timer_reg[1] |= RTC_TIMER_TI_TP; // interrupt mode
    timer_reg[1] |= source_clock << 3;   // clock source
    // timer_reg[1] = 0b00011111;

    timer_reg[0] = value;

    // write timer value
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_TIMER_VAL);
    Wire.write(timer_reg[0]);
    Wire.write(timer_reg[1]);
    Wire.endTransmission();
}

/**
 * @brief                   Returns is the timer flag on
 *
 * @returns bool            Returns true if the timer flag is on
 */
bool RTC::CheckTimerFlag()
{
    uint8_t _crtl_2 = RTC_TIMER_FLAG;

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, 1);
    _crtl_2 &= Wire.read();

    return _crtl_2;
}

/**
 * @brief                   Returns is the alarm flag on
 *
 * @returns bool            Returns true if the alarm flag is on
 */
bool RTC::CheckAlarmFlag()
{
    uint8_t _crtl_2 = RTC_ALARM_AF;

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, 1);
    _crtl_2 &= Wire.read();

    return _crtl_2;
}

/**
 * @brief                   Clears alarm flag
 */
void RTC::ClearAlarmFlag()
{
    uint8_t _crtl_2;

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, 1);

    _crtl_2 = Wire.read() & ~(RTC_ALARM_AF);

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(_crtl_2);
    Wire.endTransmission();
}

/**
 * @brief                   Clears timer flag
 */
void RTC::ClearTimerFlag()
{
    uint8_t _crtl_2;

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, 1);

    _crtl_2 = Wire.read() & ~(RTC_TIMER_FLAG);

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_2);
    Wire.write(_crtl_2);
    Wire.endTransmission();
}

/**
 * @brief                   Disables the timer
 */
void RTC::DisableTimer()
{
    uint8_t _timerMode;

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_TIMER_MODE);
    Wire.endTransmission();
    Wire.requestFrom(I2C_ADDR, 1);

    _timerMode = Wire.read() & ~(RTC_TIMER_TE);

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_TIMER_MODE);
    Wire.write(_timerMode);
    Wire.endTransmission();
}

/**
 * @brief                   Check if the RTC is already set
 *
 * @returns bool            Returns true if RTC is set, false if it's not
 */
bool RTC::IsSet()
{
    uint8_t _ramByte;
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_RAM_by);
    Wire.endTransmission();

    Wire.requestFrom(I2C_ADDR, 1);
    _ramByte = Wire.read();
    return ((_ramByte == 170) ? true : false);
}

/**
 * @brief                   Resets the timer
 */
void RTC::Reset() // datasheet 8.2.1.3.
{
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_1);
    Wire.write(0x58);
    Wire.endTransmission();
}

/**
 * @brief                   Set internal capacitor value.
 *
 * @param bool val          0 or 1 which represents 7pF or 12.5 pF.
 */
void RTC::SetInternalCapacitor(bool val)
{
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_1);
    Wire.endTransmission();

    uint8_t reg;
    Wire.requestFrom(I2C_ADDR, 1);

    if (Wire.available())
    {
        reg = Wire.read();
    }

    if (val)
    {
        reg |= (1 << 0);
    }
    else
    {
        reg &= ~(1 << 0);
    }

    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_CTRL_1);
    Wire.write(reg);
    Wire.endTransmission();
}

/**
 * @brief                   Offset used to correct the frequency of the crystal used for RTC.
 *                          8.2.3 in the datasheet.
 *
 * @param bool mode         0 - normal mode -> offset is made once every two hours.
 *                          Each LSB introduces an offset of 4.34 ppm.
 *                          1 - course mode -> offset is made every 4 minutes.
 *                          Each LSB introduces an offset of 4.069 ppm.
 *
 * @param byte offsetValue  The offset value is coded in two’s complement giving a
 *                          range of +63 LSB to -64 LSB.
 */
void RTC::SetClockOffset(bool mode, int offsetValue)
{
    // Byte for writting in the register
    uint8_t regValue;

    // Check offset value
    if (offsetValue > 63 || offsetValue < -64)
    {
        return;
    }

    // Use two's complement
    if (offsetValue < 0)
    {
        offsetValue += 128;
    }

    // Save it in the byte for register
    regValue = (byte)offsetValue;

    // Write mode in the MSB
    if (mode)
    {
        regValue |= (1 << 7); // Set MSB to 1
    }
    else
    {
        regValue &= ~(1 << 7); // Set MSB to 0
    }

    // Send to the register
    Wire.beginTransmission(I2C_ADDR);
    Wire.write(RTC_OFFSET);
    Wire.write(regValue);
    Wire.endTransmission();
}

/**
 * @brief                   Converts decimal to BCD
 *
 * @param                   uint8_t val
 *                          number which needs to be converted from decimal to Bcd value
 */
uint8_t RTC::DecToBcd(uint8_t val)
{
    return ((val / 10 * 16) + (val % 10));
}

/**
 * @brief                   Converts BCD to decimal
 *
 * @param                   uint8_t val
 *                          number which needs to be converted from Bcd to decimal value
 */
uint8_t RTC::BcdToDec(uint8_t val)
{
    return ((val / 16 * 10) + (val % 16));
}
