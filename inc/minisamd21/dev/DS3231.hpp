#pragma once
#include "minisamd21/I2C.hpp"

namespace minisamd21
{

class DS3231
{
public:
    // Static constexpr for DS3231 I2C address and register addresses
    static constexpr uint8_t ADDR = 0x68;
    static constexpr uint8_t REG_TIME = 0x00;
    static constexpr uint8_t REG_STATUS = 0x0F;
    static constexpr int32_t BASE_YEAR = 2000; // Base year for conversion

    // Time struct to hold the current time
    struct Time
    {
        uint32_t hour;
        uint32_t minute;
        uint32_t second;
        uint32_t day;
        uint32_t month;
        uint32_t year;
    };

    // Constructor that initializes the I2C object and the DS3231
    DS3231(I2C &i2c);

    // Set the time and date on the DS3231 (format: 12-hour clock)
    void SetTime(uint32_t hour, uint32_t minute, uint32_t second, uint32_t day, uint32_t month, uint32_t year);

    // Set the time using a Time struct
    void SetTime(const Time &time);

    // Read the current time from the DS3231 and return as a Time struct
    Time GetTime();

private:
    I2C &i2c_; // Reference to the I2C instance

    // Helper function to convert from BCD to decimal
    static constexpr uint8_t BcdToDec(uint8_t bcd);

    // Helper function to convert from decimal to BCD
    static constexpr uint8_t DecToBcd(uint8_t dec);
};

}