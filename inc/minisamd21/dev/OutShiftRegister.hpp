#pragma once
#include "minisamd21/Pin.hpp"
#include "cstdint"

namespace minisamd21
{
class OutShiftRegister
{
public:
    enum class Endian
    {
        MSB_FIRST,
        LSB_FIRST
    };

    OutShiftRegister(Pin data, Pin clock, Pin latch, Endian endian = Endian::MSB_FIRST)
        : data_(data), clock_(clock), latch_(latch), endian_(endian) {
          };

    // Initialize the pins
    void Init();

    void DeInit();

    // length is in bytes
    void Write(uint8_t *data, std::size_t length);

    void WriteByte(uint8_t data)
    {
        uint8_t buffer[1] = {data};
        Write(buffer, 1);
    }

private:
    Pin data_;
    Pin clock_;
    Pin latch_;
    Endian endian_;
};

}