#include <cstdint>

#include "minisamd21/AdcInput.hpp"
#include "minisamd21/I2C.hpp"
#include "minisamd21/Pin.hpp"
#include "minisamd21/Sleep.hpp"
#include "minisamd21/System.hpp"
#include "minisamd21/dev/AT24XX.hpp"
#include "minisamd21/dev/DS3231.hpp"
#include "minisamd21/dev/OutShiftRegister.hpp"

constexpr uint32_t BUTTON_PIN = 4;
constexpr uint32_t CHARGE_STATE_PIN = 5;
constexpr uint32_t LED_PIN = 23;

using namespace minisamd21;

void ButtonCallback()
{
    // Use the global LED instead of creating a new one in the interrupt
    Pin led2(Pin::PortName::PORTA, LED_PIN);
    led2.Init(Pin::Mode::OUTPUT);
    for (int i = 0; i < 4; ++i)
    {
        led2.Toggle();
        System::DelayMs(50);
    }
}

void ChargeStateCallback()
{
    // Add charge state change functionality here
}

int main()
{
    System::Init(System::ClockSource::INTERNAL_OSC);

    Pin led(Pin::PortName::PORTA, LED_PIN);
    Pin button(Pin::PortName::PORTA, BUTTON_PIN);
    Pin chargeState(Pin::PortName::PORTA, CHARGE_STATE_PIN);

    OutShiftRegister sr(Pin(Pin::PortName::PORTA, 17),
                        Pin(Pin::PortName::PORTA, 18),
                        Pin(Pin::PortName::PORTA, 19));
    sr.Init();
    sr.WriteByte(0b10101010);

    // Initialize the LED
    led.Init(Pin::Mode::OUTPUT);
    button.Init(Pin::Mode::INPUT_PULLUP);
    chargeState.Init(Pin::Mode::INPUT_PULLUP);

    Sleep::Init();

    // Set up button interrupt - should trigger when button is pressed (FALLING edge)
    button.AttachInterrupt(Pin::InterruptMode::FALLING, ButtonCallback, true);
    chargeState.AttachInterrupt(Pin::InterruptMode::FALLING, ChargeStateCallback);

    // Initialize ADC
    [[maybe_unused]] AdcInput adc(Pin(Pin::PortName::PORTA, 2));
    adc.Init();

    // Initialize I2C
    I2C i2c(I2C::Interface::TWI0);
    i2c.Init(I2C::SPEED_100KHZ);

    // Initialize DS3231
    DS3231 ds3231(i2c);

    // HH:MM:SS, DD, MM, YY
    DS3231::Time current_time = {4, 20, 0, 18, 8, 2023};
    ds3231.SetTime(current_time);

    // Initialize memory
    // AT24XX eeprom = AT24XX::AT24C32(i2c, 0x57);
    // const char *hello = "Hello, World!";
    // eeprom.Write(0x0, (uint8_t *)hello, 15);

    int32_t sleep_counter = 0;

    // Main program loop
    while (1)
    {
        int32_t delay = 500;
        led.Write(1);
        System::DelayMs(delay);
        led.Write(0);
        System::DelayMs(delay);

        [[maybe_unused]] DS3231::Time time = ds3231.GetTime();
        // char read_hello[15] = {0};
        // eeprom.Read(0x0, (uint8_t *)read_hello, 15);

        [[maybe_unused]] uint16_t adc_value = adc.Read();
        System::DelayMs(10);

        sleep_counter++;
        if (sleep_counter >= 3)
        {
            sleep_counter = 0;
            sr.DeInit();
            Sleep::SleepNow();
            sr.Init();
        }
    }
}
