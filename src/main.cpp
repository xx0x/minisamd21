#include <cstdint>

#include "minisamd21/Pin.hpp"
#include "minisamd21/System.hpp"

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

    // Main program loop
    while (1)
    {
        led.Toggle();
        int32_t delay = 500;
        if(!button.Read()){
            delay = 50;
        }
        System::DelayMs(delay);
    }
}
