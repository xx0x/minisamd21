#include <cstdint>

#include "minisamd21/I2C.hpp"
#include "minisamd21/Pin.hpp"
#include "minisamd21/System.hpp"
#include "minisamd21/dev/DS3231.hpp"

constexpr uint32_t BUTTON_PIN = 4;
constexpr uint32_t LED_PIN = 23;

using namespace minisamd21;

int main()
{
    System::Init(System::ClockSource::INTERNAL_OSC);

    Pin led(Pin::PortName::PORTA, LED_PIN);
    led.Init(Pin::Mode::OUTPUT);

    Pin button(Pin::PortName::PORTA, BUTTON_PIN);
    button.Init(Pin::Mode::INPUT_PULLUP);

    // Initialize I2C
    I2C i2c(I2C::Interface::TWI0);
    i2c.Init(I2C::SPEED_100KHZ);

    // Initialize DS3231
    DS3231 ds3231(i2c);

    // HH:MM:SS, DD, MM, YY
    DS3231::Time current_time = {4, 20, 0, 18, 8, 2023};
    ds3231.SetTime(current_time);

    // Main program loop
    while (1)
    {
        led.Toggle();
        int32_t delay = 500;
        if (!button.Read())
        {
            delay = 50;
        }

        [[maybe_unused]] DS3231::Time time = ds3231.GetTime();
        System::DelayMs(delay);
    }
}
