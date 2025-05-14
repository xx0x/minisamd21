#include "minisamd21/Pin.hpp"
#include "samd21.h"

namespace minisamd21
{

void Pin::Init(Mode mode)
{
    // Enable the port clock if it's not already enabled
    if (port_ == PortName::PORTA)
    {
        if (!(PM->APBBMASK.reg & PM_APBBMASK_PORT))
        {
            PM->APBBMASK.reg |= PM_APBBMASK_PORT;
        }
    }
    else if (port_ == PortName::PORTB)
    {
        if (!(PM->APBBMASK.reg & PM_APBBMASK_PORT))
        {
            PM->APBBMASK.reg |= PM_APBBMASK_PORT;
        }
    }

    // Configure the pin as GPIO
    PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg &= ~PORT_PINCFG_PMUXEN;

    // Set the pin direction
    if (mode == Mode::OUTPUT)
    {
        PORT->Group[static_cast<uint8_t>(port_)].DIRSET.reg = (1 << pin_); // Set pin as output
    }
    else
    {
        PORT->Group[static_cast<uint8_t>(port_)].DIRCLR.reg = (1 << pin_);            // Set pin as input
        PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg = PORT_PINCFG_INEN; // Enable input
        if (mode == Mode::INPUT_PULLUP)
        {
            PORT->Group[static_cast<uint8_t>(port_)].OUTSET.reg = (1 << pin_); // Enable pull-up
            PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg |= PORT_PINCFG_PULLEN;
        }
    }
}

void Pin::DeInit()
{
    // Disable the pin
    PORT->Group[static_cast<uint8_t>(port_)].DIRCLR.reg = (1 << pin_);              // Set pin as input
    PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg &= ~PORT_PINCFG_INEN; // Disable input
    PORT->Group[static_cast<uint8_t>(port_)].OUTCLR.reg = (1 << pin_);              // Disable pull-up
}

bool Pin::Read() const
{
    return (PORT->Group[static_cast<uint8_t>(port_)].IN.reg & (1 << pin_)) != 0;
}

void Pin::Write(bool value)
{
    if (value)
    {
        PORT->Group[static_cast<uint8_t>(port_)].OUTSET.reg = (1 << pin_);
    }
    else
    {
        PORT->Group[static_cast<uint8_t>(port_)].OUTCLR.reg = (1 << pin_);
    }
}

void Pin::Toggle()
{
    PORT->Group[static_cast<uint8_t>(port_)].OUTTGL.reg = (1 << pin_);
}

uint8_t Pin::GetPin() const
{
    return pin_;
}

Pin::PortName Pin::GetPort() const
{
    return port_;
}

void Pin::InitEIC()
{
    if (eic_initialized_)
    {
        return; // Already initialized
    }

    // Enable EIC clock in APBA
    PM->APBAMASK.reg |= PM_APBAMASK_EIC;

    // Connect GCLK0 to EIC using the correct GCLK ID
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |     // Enable GCLK
                        GCLK_CLKCTRL_GEN_GCLK0 | // Select GCLK0
                        GCLK_CLKCTRL_ID(5);      // Set ID for EIC (5)

    // Wait for synchronization
    while (GCLK->STATUS.bit.SYNCBUSY)
    {
    }

    // Reset EIC
    EIC->CTRL.bit.SWRST = 1;
    while (EIC->CTRL.bit.SWRST || EIC->STATUS.bit.SYNCBUSY)
    {
    }

    // Enable EIC
    EIC->CTRL.bit.ENABLE = 1;
    while (EIC->STATUS.bit.SYNCBUSY)
    {
    }

    // Enable NVIC for EIC
    NVIC_DisableIRQ(EIC_IRQn);
    NVIC_ClearPendingIRQ(EIC_IRQn);
    NVIC_SetPriority(EIC_IRQn, 1); // 1 = lower priority than systick (for delay to work etc)
    NVIC_EnableIRQ(EIC_IRQn);

    eic_initialized_ = true;
}

void Pin::EnableInterrupt(InterruptMode mode)
{
    // Initialize EIC if not already done (global static function)
    InitEIC();

    // Enable peripheral multiplexing for this pin
    PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg |= PORT_PINCFG_PMUXEN;

    // Set the PMUX to function A (EIC) for this pin
    uint8_t pmux_reg_pos = pin_ >> 1;         // Divide by 2 to get PMUX register
    uint8_t pmux_bit_pos = (pin_ & 0x01) * 4; // 0 or 4 based on even/odd pin

    // Clear existing PMUX setting (mask with 0xF)
    PORT->Group[static_cast<uint8_t>(port_)].PMUX[pmux_reg_pos].reg &= ~(0xF << pmux_bit_pos);

    // Set PMUX to A (0x0) for EIC function
    PORT->Group[static_cast<uint8_t>(port_)].PMUX[pmux_reg_pos].reg |= (0x0 << pmux_bit_pos);

    // Configure the sense mode in the EIC
    uint8_t config_reg_pos = pin_ >> 3;           // Divide by 8 to get CONFIG register
    uint8_t config_field_pos = (pin_ & 0x07) * 4; // 0, 4, 8, 12, 16, 20, 24, or 28 based on pin % 8

    // Clear existing sense configuration
    EIC->CONFIG[config_reg_pos].reg &= ~(0x7 << config_field_pos);

    // Set interrupt sense mode
    if (mode == InterruptMode::RISING)
    {
        EIC->CONFIG[config_reg_pos].reg |= (EIC_CONFIG_SENSE0_RISE_Val << config_field_pos);
    }
    else if (mode == InterruptMode::FALLING)
    {
        EIC->CONFIG[config_reg_pos].reg |= (EIC_CONFIG_SENSE0_FALL_Val << config_field_pos);
    }
    else if (mode == InterruptMode::CHANGE)
    {
        EIC->CONFIG[config_reg_pos].reg |= (EIC_CONFIG_SENSE0_BOTH_Val << config_field_pos);
    }

    // Make sure EIC is enabled
    if (!EIC->CTRL.bit.ENABLE)
    {
        EIC->CTRL.bit.ENABLE = 1;
        while (EIC->STATUS.bit.SYNCBUSY)
        {
        }
    }

    // Enable interrupt for this specific pin
    EIC->INTENSET.reg = EIC_INTENSET_EXTINT(1 << pin_);
}

void Pin::AttachInterrupt(Pin::InterruptMode mode, Callback callback, bool wakeup)
{
    interrupt_callbacks_[pin_] = callback;
    interrupt_attached_[pin_] = true;

    EnableInterrupt(mode);

    // Enable wakeup capability if requested
    if (wakeup)
    {
        // Sync function to wait for GCLK
        static auto waitForSync = []()
        {
            while (GCLK->STATUS.bit.SYNCBUSY)
            {
            }
        };

        // Configure the clock for EIC
        GCLK->CLKCTRL.bit.CLKEN = 0;
        waitForSync();

        GCLK->CLKCTRL.reg = (uint16_t)(GCLK_CLKCTRL_CLKEN | GCLK_CLKCTRL_GEN_GCLK6 | GCLK_CLKCTRL_ID(5));
        waitForSync();

        GCLK->GENCTRL.reg = (GCLK_GENCTRL_GENEN | GCLK_GENCTRL_SRC_OSCULP32K | GCLK_GENCTRL_ID(6));
        waitForSync();

        GCLK->GENCTRL.bit.RUNSTDBY = 1;
        waitForSync();

        EIC->WAKEUP.reg |= (1 << pin_);
    }
}

void Pin::InterruptHandler(uint8_t pinNumber)
{
    if (interrupt_attached_[pinNumber] && interrupt_callbacks_[pinNumber] != nullptr)
    {
        interrupt_callbacks_[pinNumber]();
    }
}

} // namespace minisamd21

extern "C" void EIC_Handler()
{
    // Get the interrupt flags to determine which pins triggered
    uint32_t flags = EIC->INTFLAG.reg;

    // Process all pins that have pending interrupts
    for (int i = 0; i < 32; ++i)
    {
        if (flags & (1 << i))
        {
            // Call the handler for this pin
            minisamd21::Pin::InterruptHandler(i);

            // Clear the interrupt flag by writing 1 to it
            EIC->INTFLAG.reg = (1 << i);
        }
    }
}
