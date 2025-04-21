#pragma once
#include "samd21.h"

namespace minisamd21
{

/**
 * @brief Class for handling GPIO pins on the SAMD21 microcontroller.
 *
 * This class provides methods to initialize, read, and write to GPIO pins on the SAMD21 microcontroller.
 * It supports setting the pin mode as INPUT, INPUT_PULLUP, or OUTPUT, and reading or writing the pin state.
 *
 */
class Pin
{
public:
    enum class PortName
    {
        PORTA,
        PORTB
    };

    // Modes
    enum class Mode
    {
        INPUT,
        INPUT_PULLUP,
        OUTPUT
    };

    // Constructor
    Pin(PortName port, uint8_t pin);

    // Static function to create and initialize a pin
    static Pin Create(PortName port, uint8_t pin, Mode mode);

    // Initialize the pin
    void Init(Mode mode);

    // Read the pin (returns true for HIGH, false for LOW)
    bool Read() const;

    // Set the pin HIGH or LOW
    void Write(bool value);

    // Toggle the pin
    void Toggle();

private:
    PortName port_;
    uint8_t pin_;
};

}