#include <stdint.h>

#define LED_PIN 23

#include "minisamd21/Pin.hpp"
#include "minisamd21/System.hpp"

#include "samd21.h"
using namespace minisamd21;


int main()
{
    System::Init(System::ClockSource::INTERNAL_OSC);

    Pin led(Pin::PortName::PORTA, LED_PIN);
    led.Init(Pin::Mode::OUTPUT);

    Pin button(Pin::PortName::PORTA, 4);
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
