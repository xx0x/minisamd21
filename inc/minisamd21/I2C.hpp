#pragma once
#include "samd21.h"
#include <cstdint>

namespace minisamd21
{

class I2C
{
public:
    enum class Interface
    {
        TWI0 = 0, // PA08 = SDA, PA09 = SCL
        TWI1 = 1, // PA22 = SDA, PA23 = SCL
    };
    
    void Init(Interface iface, uint32_t baud);
    void Write(uint8_t address, uint8_t *data, uint32_t length, bool nostop = false);
    void Read(uint8_t address, uint8_t *data, uint32_t length);
    void ReadRegister(uint16_t address, uint8_t register_address, uint8_t address_size, uint8_t *data, bool nostop = false);
    void WriteRegisters(uint16_t address, uint8_t register_address, uint8_t *data, uint32_t length, bool nostop = false);

private:
    Sercom *sercom_;
    uint32_t baud_;

    void EnablePeripheral();
};

} // namespace minisamd21
