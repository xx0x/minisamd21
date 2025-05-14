#include "minisamd21/PwmOutput.hpp"

#include <algorithm>

using namespace minisamd21;

/**
 * The implementation is based on the Arduino SAMD core.
 * The original code is licensed under the LGPL license.
 * Copyright (c) 2014 Arduino LLC. All right reserved.
 */
void PwmOutput::MapPinToTimer()
{
    uint8_t pin_no = pin_.GetPin();
    Pin::PortName port = pin_.GetPort();

    // Default to no timer
    timer_instance_ = nullptr;
    timer_type_ = TimerType::NONE;
    timer_channel_ = 0;

    // Search through the mapping table
    for (const auto &mapping : TIMER_MAPPINGS)
    {
        if (mapping.port == port && mapping.pin == pin_no)
        {
            // Configure pin multiplexer for the timer function
            PORT->Group[static_cast<uint8_t>(port)].PINCFG[pin_no].bit.PMUXEN = 1;

            if (pin_no & 1)
            { // Odd pin number
                PORT->Group[static_cast<uint8_t>(port)].PMUX[pin_no >> 1].bit.PMUXO = mapping.mux_function;
            }
            else
            { // Even pin number
                PORT->Group[static_cast<uint8_t>(port)].PMUX[pin_no >> 1].bit.PMUXE = mapping.mux_function;
            }

            // Store timer information
            timer_type_ = mapping.type;
            timer_channel_ = mapping.channel;

            // Get pointer to the appropriate timer instance
            if (timer_type_ == TimerType::TCC)
            {
                switch (mapping.instance)
                {
                case 0:
                    timer_instance_ = TCC0;
                    break;
                case 1:
                    timer_instance_ = TCC1;
                    break;
                case 2:
                    timer_instance_ = TCC2;
                    break;
                }
            }
            else if (timer_type_ == TimerType::TC)
            {
                switch (mapping.instance)
                {
                case 3:
                    timer_instance_ = TC3;
                    break;
                case 4:
                    timer_instance_ = TC4;
                    break;
                case 5:
                    timer_instance_ = TC5;
                    break;
#ifdef __SAMD21J18A__
                case 6:
                    timer_instance_ = TC6;
                    break;
#endif
                }
            }

            // Found a match, exit the loop
            break;
        }
    }

    if (timer_instance_ == nullptr)
    {
        // Error: No PWM capability found for this pin
        while (1)
        {
            // Hang in an infinite loop if in debug mode
        }
    }
}

void PwmOutput::SyncTC(Tc *TCx)
{
    while (TCx->COUNT16.STATUS.bit.SYNCBUSY)
    {
        // Wait for synchronization to complete
    }
}

void PwmOutput::SyncTCC(Tcc *TCCx)
{
    while (TCCx->SYNCBUSY.reg & TCC_SYNCBUSY_MASK)
    {
        // Wait for synchronization to complete
    }
}

void PwmOutput::Init(uint32_t frequency)
{
    // Limit frequency to maximum
    frequency_ = std::min(frequency, MAX_FREQUENCY);

    // Set the pin as output
    pin_.Init(Pin::Mode::OUTPUT);

    // Map pin to the appropriate timer peripheral
    MapPinToTimer();

    // Check if valid timer was found
    if (timer_instance_ == nullptr)
    {
        return;
    }

    // Enable appropriate peripheral clock based on timer type and instance
    uint8_t timer_num;
    if (timer_type_ == TimerType::TCC)
    {
        Tcc *tcc = static_cast<Tcc *>(timer_instance_);
        timer_num = (tcc == TCC0) ? 0 : ((tcc == TCC1) ? 1 : 2);

        // Enable TCC clock
        switch (timer_num)
        {
        case 0:
            PM->APBCMASK.reg |= PM_APBCMASK_TCC0;
            break;
        case 1:
            PM->APBCMASK.reg |= PM_APBCMASK_TCC1;
            break;
        case 2:
            PM->APBCMASK.reg |= PM_APBCMASK_TCC2;
            break;
        }
    }
    else if (timer_type_ == TimerType::TC)
    {
        Tc *tc = static_cast<Tc *>(timer_instance_);
        timer_num = (tc == TC3) ? 3 : ((tc == TC4) ? 4 : 5); // Only TC3-5 are used

        // Enable TC clock
        switch (timer_num)
        {
        case 3:
            PM->APBCMASK.reg |= PM_APBCMASK_TC3;
            break;
        case 4:
            PM->APBCMASK.reg |= PM_APBCMASK_TC4;
            break;
        case 5:
            PM->APBCMASK.reg |= PM_APBCMASK_TC5;
            break;
#ifdef __SAMD21J18A__
        case 6:
            PM->APBCMASK.reg |= PM_APBCMASK_TC6;
            break;
#endif
        }
    }

    // Enable GCLK for TCC and TC
    if (!timer_enabled_[timer_num])
    {
        timer_enabled_[timer_num] = true;

        // Determine GCLK_CLKCTRL_ID based on the timer
        uint16_t GCLK_ID;

        if (timer_num <= 2)
        {
            // TCC0, TCC1, TCC2
            GCLK_ID = timer_num <= 1 ? GCLK_CLKCTRL_ID_TCC0_TCC1 : GCLK_CLKCTRL_ID_TCC2_TC3;
        }
        else
        {
            // TC3, TC4, TC5, TC6
#ifdef __SAMD21J18A__
            if (timer_num == 6) {
                GCLK_ID = GCLK_CLKCTRL_ID_TC6_TC7;
            } else
#endif
            {
                GCLK_ID = timer_num == 3 ? GCLK_CLKCTRL_ID_TCC2_TC3 : GCLK_CLKCTRL_ID_TC4_TC5;
            }
        }

        // Connect the timer to GCLK0 (48MHz)
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
                            GCLK_CLKCTRL_GEN_GCLK0 |
                            GCLK_ID;

        while (GCLK->STATUS.bit.SYNCBUSY)
        {
            // Wait for synchronization
        }

        // Configure the timer
        if (timer_type_ == TimerType::TCC)
        {
            // Configure TCC
            Tcc *tcc = static_cast<Tcc *>(timer_instance_);

            // Disable TCC
            tcc->CTRLA.bit.ENABLE = 0;
            SyncTCC(tcc);

            // Set normal PWM mode
            tcc->WAVE.reg = TCC_WAVE_WAVEGEN_NPWM;
            SyncTCC(tcc);

            // Calculate and set the PER register based on frequency
            uint32_t period = System::FREQUENCY / frequency_; // 48MHz / desired frequency
            tcc->PER.reg = period - 1;
            SyncTCC(tcc);

            // Set initial duty cycle to 0
            tcc->CC[timer_channel_].reg = 0;
            SyncTCC(tcc);

            // Enable TCC
            tcc->CTRLA.reg |= TCC_CTRLA_ENABLE;
            SyncTCC(tcc);
        }
        else if (timer_type_ == TimerType::TC)
        {
            // Configure TC
            Tc *tc = static_cast<Tc *>(timer_instance_);

            // Disable TC
            tc->COUNT16.CTRLA.bit.ENABLE = 0;
            SyncTC(tc);

            // Set Timer counter Mode to 16 bits, normal PWM
            tc->COUNT16.CTRLA.reg |= TC_CTRLA_MODE_COUNT16 | TC_CTRLA_WAVEGEN_NPWM;
            SyncTC(tc);

            // Calculate and set the PER register based on frequency
            uint32_t period = System::FREQUENCY / frequency_; // 48MHz / desired frequency
            tc->COUNT16.CC[0].reg = period - 1;
            SyncTC(tc);

            // Set initial duty cycle to 0
            tc->COUNT16.CC[timer_channel_].reg = 0;
            SyncTC(tc);

            // Enable TC
            tc->COUNT16.CTRLA.bit.ENABLE = 1;
            SyncTC(tc);
        }
    }

    // Set initial duty cycle to 0.0
    Write(0.0f);
}

void PwmOutput::Write(float duty_cycle)
{
    // Constrain duty cycle to 0.0-1.0 range
    duty_cycle_ = std::clamp(duty_cycle, 0.0f, 1.0f);

    // Check if valid timer was found
    if (timer_instance_ == nullptr)
    {
        return;
    }

    if (timer_type_ == TimerType::TCC)
    {
        // Handle TCC-based PWM
        Tcc *tcc = static_cast<Tcc *>(timer_instance_);

        // Calculate the compare value based on duty cycle
        uint32_t value = static_cast<uint32_t>((tcc->PER.reg + 1) * duty_cycle_);

        // Update the compare register with the new duty cycle value
        tcc->CTRLBSET.bit.LUPD = 1;
        SyncTCC(tcc);

        tcc->CCB[timer_channel_].reg = value;
        SyncTCC(tcc);

        tcc->CTRLBCLR.bit.LUPD = 1;
        SyncTCC(tcc);
    }
    else if (timer_type_ == TimerType::TC)
    {
        // Handle TC-based PWM
        Tc *tc = static_cast<Tc *>(timer_instance_);

        // Calculate the compare value based on duty cycle
        uint32_t value = static_cast<uint32_t>((tc->COUNT16.CC[0].reg + 1) * duty_cycle_);

        // Update the compare register with the new duty cycle value
        tc->COUNT16.CC[timer_channel_].reg = value;
        SyncTC(tc);
    }
}