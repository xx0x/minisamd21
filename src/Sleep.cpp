#include "minisamd21/Sleep.hpp"

#include <samd21.h>

namespace minisamd21
{

void Sleep::Init()
{
    // Enable sleep mode
    SCB->SCR |= SCB_SCR_SLEEPDEEP_Msk;
}

void Sleep::SleepNow()
{
    __DSB();
    __WFI();
}

} // namespace minisamd21
