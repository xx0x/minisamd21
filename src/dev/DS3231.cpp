#include "minisamd21/dev/DS3231.hpp"
#include <algorithm>

namespace minisamd21
{

DS3231::DS3231(I2C &i2c) : i2c_(i2c) {}

void DS3231::SetTime(uint32_t hour, uint32_t minute, uint32_t second, uint32_t day, uint32_t month, uint32_t year)
{
    // Ensure year is between 0 and 99
    int32_t year_fixed = std::clamp(static_cast<int>(year - BASE_YEAR), 0, 99);
    uint8_t data[8] = {0};

    // Address
    data[0] = REG_TIME;

    // Set the time (in BCD format)
    data[1] = DecToBcd(second);          // Seconds
    data[2] = DecToBcd(minute);          // Minutes
    data[3] = DecToBcd(hour) & 0b111111; // Hours (no AM/PM bit, 24-hour format)

    // Set the date (in BCD format)
    data[4] = 0;                    // Day of the week (not used)
    data[5] = DecToBcd(day);        // Month
    data[6] = DecToBcd(month);      // Month
    data[7] = DecToBcd(year_fixed); // Year 0-99

    // Write the time and date to the DS3231
    i2c_.Write(ADDR, data, 8);
}

void DS3231::SetTime(const Time &time)
{
    SetTime(time.hour, time.minute, time.second, time.day, time.month, time.year);
}

DS3231::Time DS3231::GetTime()
{
    uint8_t data[7] = {0};

    // Read the time from the DS3231
    i2c_.ReadRegisters(ADDR, REG_TIME, 1, data, 7);

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