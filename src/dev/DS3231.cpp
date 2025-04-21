#include "minisamd21/dev/DS3231.hpp"

namespace minisamd21
{

DS3231::DS3231(I2C &i2c) : i2c_(i2c) {}

void DS3231::SetTime(uint8_t hour, uint8_t minute, uint8_t second, uint8_t day, uint8_t month, uint8_t year)
{
    uint8_t data[7] = {0};

    // Start at the TIME register (0x00)
    data[0] = REG_TIME;

    // Set the time (in BCD format)
    data[1] = DecToBcd(second); // Seconds
    data[2] = DecToBcd(minute); // Minutes
    data[3] = DecToBcd(hour);   // Hours

    // Set the date (in BCD format)
    data[4] = DecToBcd(day);   // Day
    data[5] = DecToBcd(month); // Month
    data[6] = DecToBcd(year);  // Year

    // Write the time and date to the DS3231
    i2c_.Write(ADDR, data, 7);
}

void DS3231::SetTime(const Time &time)
{
    SetTime(time.hour, time.minute, time.second, time.day, time.month, time.year);
}

DS3231::Time DS3231::GetTime()
{
    uint8_t data[7] = {0};

    // Read the time from the DS3231
    i2c_.Read(ADDR, data, 7);

    // Convert BCD to decimal and store in the Time struct
    Time current_time;
    current_time.second = BcdToDec(data[1]);
    current_time.minute = BcdToDec(data[2]);
    current_time.hour = BcdToDec(data[3]);
    current_time.day = BcdToDec(data[4]);
    current_time.month = BcdToDec(data[5]);
    current_time.year = BcdToDec(data[6]);

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