#include "minisamd21/dev/AT24XX.hpp"
#include "minisamd21/System.hpp"
#include <algorithm>

namespace minisamd21
{

AT24XX::AT24XX(I2C &i2c, uint8_t device_address, uint32_t memory_size, uint32_t page_size, uint32_t address_size)
    : device_address_(device_address),
      memory_size_(memory_size),
      page_size_(page_size),
      address_size_(address_size),
      i2c_(i2c)
{
}

bool AT24XX::WriteByte(uint16_t address, uint8_t data)
{
    return Write(address, &data, 1);
}

bool AT24XX::Write(uint16_t address, uint8_t *data, uint32_t length)
{
    uint32_t bytes_written = 0;
    while (bytes_written < length)
    {
        uint32_t remaining_in_page = page_size_ - (address % page_size_);
        uint32_t write_length = std::min(length - bytes_written, remaining_in_page);

        if (!WritePage(address + bytes_written, data + bytes_written, write_length))
        {
            return false;
        }

        bytes_written += write_length;
        if (!WaitForWriteCompletion())
        {
            return false;
        }
    }
    return true;
}

bool AT24XX::ReadByte(uint16_t address, uint8_t &data)
{
    return Read(address, &data, 1);
}

bool AT24XX::Read(uint16_t address, uint8_t *data, uint32_t length)
{
    i2c_.ReadRegisters(device_address_, address, address_size_, data, length);
    return true;
}

bool AT24XX::WritePage(uint16_t address, uint8_t *data, uint32_t length)
{
    i2c_.WriteRegisters(device_address_, address, address_size_, data, length);
    return true;
}

bool AT24XX::ReadPage(uint16_t address, uint8_t *data, uint32_t length)
{
    i2c_.ReadRegisters(device_address_, address, address_size_, data, length);
    return true;
}

bool AT24XX::WaitForWriteCompletion()
{
    System::DelayMs(10); // EEPROM write time (10ms should be enough for most EEPROMs)
    return true;
}

AT24XX AT24XX::AT24C32(I2C &i2c, uint8_t device_address)
{
    return AT24XX(i2c, device_address, 4096, 32, 2); // 4KB (4096 bytes), 32-byte page size
}

AT24XX AT24XX::AT24C256(I2C &i2c, uint8_t device_address)
{
    return AT24XX(i2c, device_address, 32768, 64, 2); // 32KB (32768 bytes), 64-byte page size
}

AT24XX AT24XX::AT24C512(I2C &i2c, uint8_t device_address)
{
    return AT24XX(i2c, device_address, 65536, 128, 2); // 64KB (65536 bytes), 128-byte page size
}

} // namespace minisamd21
