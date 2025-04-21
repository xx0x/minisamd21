#include <stdint.h>

#define LED_PIN 23

#include "minisamd21/Pin.hpp"
#include "minisamd21/System.hpp"

#include "samd21.h"
using namespace minisamd21;

void delay_ms(uint32_t ms)
{
    // For 48MHz clock (48,000 cycles per millisecond)
    SysTick->LOAD = 48000 - 1;
    SysTick->VAL = 0;
    SysTick->CTRL = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    for (uint32_t i = 0; i < ms; ++i)
    {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0)
            ;
    }

    SysTick->CTRL = 0;
}


int main()
{
    System::Init(System::ClockSource::INTERNAL_OSC);

    Pin led(Pin::PortName::PORTA, LED_PIN);
    led.Init(Pin::Mode::OUTPUT);

    /* Main program loop */
    while (1)
    {
        led.Toggle();
        delay_ms(100);
    }
}
