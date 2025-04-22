#include "minisamd21/AdcInput.hpp"
#include "samd21.h"
#include <cassert> // Include for assert()

using namespace minisamd21;

uint8_t AdcInput::MapPinToChannel(Pin::PortName port, uint8_t pin)
{
    switch (port)
    {
    case Pin::PortName::PORTA:
        switch (pin)
        {
        case 2:
            return 0; // PA02 = AIN0
        case 3:
            return 1; // PA03 = AIN1
        case 4:
            return 4; // PA04 = AIN4
        case 5:
            return 5; // PA05 = AIN5
        case 6:
            return 6; // PA06 = AIN6
        case 7:
            return 7; // PA07 = AIN7
        case 8:
            return 16; // PA08 = AIN16
        case 9:
            return 17; // PA09 = AIN17
        // ...add more as needed...
        default:
            return 0xFF; // Invalid channel
        }
    case Pin::PortName::PORTB:
        switch (pin)
        {
        case 0:
            return 8; // PB00 = AIN8
        case 1:
            return 9; // PB01 = AIN9
        case 2:
            return 10; // PB02 = AIN10
        // ...add more as needed...
        default:
            return 0xFF; // Invalid channel
        }
    default:
        return 0xFF; // Invalid port
    }
}

AdcInput::AdcInput(Pin::PortName port, uint8_t pin) : port_(port), pin_(pin)
{
    channel_ = MapPinToChannel(port, pin);
    if (channel_ == 0xFF)
    {
        while (1)
        {
            // fail, invalid pin
        }
    }
}

AdcInput AdcInput::Create(Pin::PortName port, uint8_t pin, Resolution res, Reference ref)
{
    AdcInput adc_instance(port, pin);
    adc_instance.Init(res, ref);
    return adc_instance;
}

void AdcInput::Init(Resolution res, Reference ref)
{
    // Enable the APBC clock for the ADC
    PM->APBCMASK.reg |= PM_APBCMASK_ADC;

    // Set up the GCLK for ADC
    GCLK->CLKCTRL.reg = GCLK_CLKCTRL_CLKEN |
                        GCLK_CLKCTRL_GEN_GCLK0 |
                        GCLK_CLKCTRL_ID_ADC;
    while (GCLK->STATUS.bit.SYNCBUSY)
        ;

    // Reset the ADC
    ADC->CTRLA.bit.SWRST = 1;
    SyncBusy();
    while (ADC->CTRLA.bit.SWRST)
        ;

    // Set reference
    SetReference(ref);

    // Set resolution
    SetResolution(res);

    // Set clock prescaler (divide input clock by 512)
    ADC->CTRLB.reg |= ADC_CTRLB_PRESCALER_DIV512;

    // Set sample time length
    ADC->SAMPCTRL.reg = ADC_SAMPCTRL_SAMPLEN(64);

    // Configure pin as analog input
    PORT->Group[static_cast<uint8_t>(port_)].DIRCLR.reg = (1 << pin_);    // Set as input
    PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].bit.PMUXEN = 1; // Enable peripheral mux

    if (pin_ & 1) // Odd pin number
    {
        PORT->Group[static_cast<uint8_t>(port_)].PMUX[pin_ >> 1].bit.PMUXO = 0x1; // Function B (ADC)
    }
    else // Even pin number
    {
        PORT->Group[static_cast<uint8_t>(port_)].PMUX[pin_ >> 1].bit.PMUXE = 0x1; // Function B (ADC)
    }
    // Enable ADC
    ADC->CTRLA.bit.ENABLE = 1;
    SyncBusy();
}

uint16_t AdcInput::Read() const
{
    // Select the ADC input channel without overriding the gain
    ADC->INPUTCTRL.reg = (ADC->INPUTCTRL.reg & ~ADC_INPUTCTRL_MUXPOS_Msk) |
                         ADC_INPUTCTRL_MUXPOS(channel_);
    SyncBusy();

    // Start the conversion
    ADC->SWTRIG.bit.START = 1;
    SyncBusy();

    // Wait for conversion to complete
    while (!ADC->INTFLAG.bit.RESRDY)
        ;

    // Read the result
    return ADC->RESULT.reg;
}

void AdcInput::SetReference(Reference ref)
{
    // No negative input (internal ground)
    ADC->INPUTCTRL.reg = ADC_INPUTCTRL_MUXNEG_GND;

    // Clear the current reference setting
    ADC->REFCTRL.reg &= ~ADC_REFCTRL_REFSEL_Msk;

    // Set the new reference and configure gain
    switch (ref)
    {
    case Reference::INT1V:
        ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;
        ADC->REFCTRL.reg |= ADC_REFCTRL_REFSEL_INT1V;
        break;
    case Reference::INTVCC0:
        ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;
        ADC->REFCTRL.reg |= ADC_REFCTRL_REFSEL_INTVCC0;
        break;
    case Reference::INTVCC1:
        ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_DIV2_Val;
        ADC->REFCTRL.reg |= ADC_REFCTRL_REFSEL_INTVCC1;
        break;
    case Reference::AREF:
        ADC->INPUTCTRL.bit.GAIN = ADC_INPUTCTRL_GAIN_1X_Val;
        ADC->REFCTRL.reg |= ADC_REFCTRL_REFSEL_AREFA;
        break;
    }

    SyncBusy();
}

void AdcInput::SetResolution(Resolution res)
{
    // One sample only and no adjustment
    ADC->AVGCTRL.reg = ADC_AVGCTRL_SAMPLENUM_1 |
                       ADC_AVGCTRL_ADJRES(0x0ul);

    // Clear the current resolution setting
    ADC->CTRLB.reg &= ~ADC_CTRLB_RESSEL_Msk;

    // Set the new resolution
    switch (res)
    {
    case Resolution::BIT12:
        ADC->CTRLB.reg |= ADC_CTRLB_RESSEL_12BIT;
        break;
    case Resolution::BIT10:
        ADC->CTRLB.reg |= ADC_CTRLB_RESSEL_10BIT;
        break;
    case Resolution::BIT8:
        ADC->CTRLB.reg |= ADC_CTRLB_RESSEL_8BIT;
        break;
    }

    SyncBusy();
}