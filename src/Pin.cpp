#include "minisamd21/Pin.hpp"
#include "samd21.h" // Include the header file that defines PORT

using namespace minisamd21;

Pin::Pin(PortName port, uint8_t pin) : port_(port), pin_(pin) {}

Pin Pin::Create(PortName port, uint8_t pin, Mode mode)
{
    Pin pin_instance(port, pin);
    pin_instance.Init(mode);
    return pin_instance;
}

void Pin::Init(Mode mode)
{
    if (mode == Mode::OUTPUT)
    {
        PORT->Group[static_cast<uint8_t>(port_)].DIRSET.reg = (1 << pin_); // Set pin as output
    }
    else
    {
        PORT->Group[static_cast<uint8_t>(port_)].DIRCLR.reg = (1 << pin_);       // Set pin as input
        PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg = PORT_PINCFG_INEN; // Enable input
        if (mode == Mode::INPUT_PULLUP)
        {
            PORT->Group[static_cast<uint8_t>(port_)].OUTSET.reg = (1 << pin_); // Enable pull-up
            PORT->Group[static_cast<uint8_t>(port_)].PINCFG[pin_].reg |= PORT_PINCFG_PULLEN;
        }
    }
}

bool Pin::Read() const
{
    return (PORT->Group[static_cast<uint8_t>(port_)].IN.reg & (1 << pin_)) != 0;
}

void Pin::Write(bool value) const
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

void Pin::Toggle() const
{
    PORT->Group[static_cast<uint8_t>(port_)].OUTTGL.reg = (1 << pin_);
}