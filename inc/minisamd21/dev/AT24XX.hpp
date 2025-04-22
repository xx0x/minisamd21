#pragma once
#include "minisamd21/I2C.hpp"

namespace minisamd21
{

/**
 * @brief Class for handling AT24-style EEPROMs via I2C communication.
 *
 * This class provides methods for reading and writing single bytes or multiple bytes
 * to an AT24-series EEPROM. It utilizes the I2C class for communication.
 *
 * Typical EEPROMs like AT24 are accessed by I2C, and this class supports
 * communication with them by specifying the I2C address and implementing common
 * operations such as byte writes, block writes, and block reads.
 */
class AT24XX
{
public:
    static constexpr uint8_t DEFAULT_ADDRESS = 0x50; ///< Default I2C address for AT24XX EEPROMs

    /**
     * @brief Constructor to initialize the AT24XX with I2C and device address.
     *
     * @param i2c The I2C interface to use.
     * @param device_address The I2C address of the EEPROM device (7-bit address).
     * @param memory_size The total memory size of the EEPROM in bytes.
     * @param page_size The size of each page for page writes (usually 64 bytes).
     * @param address_size Address size in bytes (1 or 2). Default is 2 for larger EEPROMs.
     */
    AT24XX(I2C &i2c, uint8_t device_address, uint32_t memory_size, uint32_t page_size = 64, uint32_t address_size = 2);

    /**
     * @brief Write a single byte to the EEPROM.
     *
     * This function writes a single byte of data to the EEPROM at the specified
     * memory address.
     *
     * @param address The address within the EEPROM to write to.
     * @param data The byte of data to write.
     * @return True if the write was successful, false otherwise.
     */
    bool WriteByte(uint16_t address, uint8_t data);

    /**
     * @brief Write multiple bytes to the EEPROM, supporting page writes.
     *
     * This function writes a block of data starting from a specified address in the EEPROM.
     * Data is written in pages if the length exceeds the page size.
     *
     * @param address The starting address within the EEPROM to write to.
     * @param data Pointer to the buffer containing the data to write.
     * @param length The number of bytes to write.
     * @return True if the write was successful, false otherwise.
     */
    bool Write(uint16_t address, uint8_t *data, uint32_t length);

    /**
     * @brief Read a single byte from the EEPROM.
     *
     * This function reads a single byte of data from the EEPROM at the specified
     * memory address.
     *
     * @param address The address within the EEPROM to read from.
     * @param data Pointer to the byte to store the read data.
     * @return True if the read was successful, false otherwise.
     */
    bool ReadByte(uint16_t address, uint8_t &data);

    /**
     * @brief Read multiple bytes from the EEPROM.
     *
     * This function reads a block of data starting from a specified address in the EEPROM.
     *
     * @param address The starting address within the EEPROM to read from.
     * @param data Pointer to the buffer where the read data will be stored.
     * @param length The number of bytes to read.
     * @return True if the read was successful, false otherwise.
     */
    bool Read(uint16_t address, uint8_t *data, uint32_t length);

    static AT24XX AT24C32(I2C &i2c, uint8_t device_address = DEFAULT_ADDRESS);
    static AT24XX AT24C256(I2C &i2c, uint8_t device_address = DEFAULT_ADDRESS);
    static AT24XX AT24C512(I2C &i2c, uint8_t device_address = DEFAULT_ADDRESS);

private:
    bool WritePage(uint16_t address, uint8_t *data, uint32_t length);
    bool ReadPage(uint16_t address, uint8_t *data, uint32_t length);
    bool WaitForWriteCompletion();

    uint8_t device_address_;
    uint32_t memory_size_;
    uint32_t page_size_;
    uint32_t address_size_;
    I2C &i2c_; // Reference to the I2C instance
};

}