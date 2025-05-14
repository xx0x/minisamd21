#include "minisamd21/dev/OutShiftRegister.hpp"
#include "minisamd21/System.hpp"

namespace minisamd21
{

void OutShiftRegister::Init()
{
    data_.Init(Pin::Mode::OUTPUT);
    clock_.Init(Pin::Mode::OUTPUT);
    latch_.Init(Pin::Mode::OUTPUT);
    data_.Write(false);
    clock_.Write(false);
    latch_.Write(false);
}

void OutShiftRegister::DeInit()
{
    data_.DeInit();
    clock_.DeInit();
    latch_.DeInit();
}

void OutShiftRegister::Write(uint8_t *data, std::size_t length)
{
    const std::size_t total_bits = length * 8;

    latch_.Write(false); // Begin latch
    System::DelayUs(2);  // Small delay to ensure latch works
    for (std::size_t i = 0; i < total_bits; ++i)
    {
        bool bit;
        if (endian_ == Endian::MSB_FIRST)
        {
            bit = (data[i / 8] & (1 << (7 - (i % 8)))) != 0;
        }
        else
        {
            bit = (data[i / 8] & (1 << (i % 8))) != 0;
        }

        data_.Write(bit);
        clock_.Write(true);
        System::DelayUs(2); // Small delay to ensure clock pulse
        clock_.Write(false);
    }

    latch_.Write(true); // End latch
}

} // namespace minisamd21