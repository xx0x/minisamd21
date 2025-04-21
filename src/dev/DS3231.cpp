#include "minisamd21/dev/DS3231.hpp"

namespace minisamd21
{

DS3231::DS3231(I2C &i2c) : i2c_(i2c) {}

void DS3231::SetTime(uint32_t hour, uint32_t minute, uint32_t second, uint32_t day, uint32_t month, uint32_t year)
{
    int32_t year_fixed = year - BASE_YEAR;
    if (year_fixed < 0)
    {
        return;
    }
    uint8_t data[7] = {0};

    // Set the time (in BCD format)
    data[0] = DecToBcd(second);          // Seconds
    data[1] = DecToBcd(minute);          // Minutes
    data[2] = DecToBcd(hour) & 0b111111; // Hours (no AM/PM bit, 24-hour format)

    // Set the date (in BCD format)
    data[3] = 0;                    // Day of the week (not used)
    data[4] = DecToBcd(day);        // Month
    data[5] = DecToBcd(month);      // Month
    data[6] = DecToBcd(year_fixed); // Year 0-99

    // Write the time and date to the DS3231
    i2c_.WriteRegisters(ADDR, REG_TIME, 1, data, 7);
}

void DS3231::SetTime(const Time &time)
{
    SetTime(time.hour, time.minute, time.second, time.day, time.month, time.year);
}

DS3231::Time DS3231::GetTime()
{
    uint8_t data[7] = {0};
    uint8_t buff[2] = {REG_TIME};

    // Read the time from the DS3231
    i2c_.Write(ADDR, buff, 1, true); // Send the register address
    i2c_.Read(ADDR, data, 7);        // Read the 7 bytes of time data

    // Convert BCD to decimal and store in the Time struct
    Time current_time;
    current_time.second = BcdToDec(data[0]);
    current_time.minute = BcdToDec(data[1]);
    current_time.hour = BcdToDec(data[2]);
    // Skipping day of the week (data[3])
    current_time.day = BcdToDec(data[4]);
    current_time.month = BcdToDec(data[5]);
    current_time.year = BcdToDec(data[6]) + BASE_YEAR; // Convert to full year

    return current_time;
}

constexpr uint8_t DS3231::BcdToDec(uint8_t bcd)
{
    return (bcd & 0x0F) + ((bcd >> 4) * 10);
}

constexpr uint8_t DS3231::DecToBcd(uint8_t dec)
{
    return ((dec / 10) << 4) + (dec % 10);
}

}