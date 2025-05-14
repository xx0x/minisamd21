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
    // Callback type for interrupt handlers
    using Callback = void (*)();

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

    enum class InterruptMode
    {
        FALLING,
        RISING,
        CHANGE
    };

    // Constructor
    Pin(PortName port, uint8_t pin) : port_(port), pin_(pin) {}

    // Initialize the pin
    void Init(Mode mode);

    void DeInit();

    // Read the pin (returns true for HIGH, false for LOW)
    bool Read() const;

    // Set the pin HIGH or LOW
    void Write(bool value);

    // Toggle the pin
    void Toggle();

    // Get the pin number
    uint8_t GetPin() const;

    // Get the port name
    PortName GetPort() const;

    // Attach an interrupt to the pin
    void AttachInterrupt(Pin::InterruptMode mode, Callback callback, bool wakeup = false);

    // Called by the EIC_Handler
    // You should not call this directly
    static void InterruptHandler(uint8_t pinNumber);

private:
    PortName port_;
    uint8_t pin_;

    // Enable interrupt
    void EnableInterrupt(InterruptMode mode);

    static inline Callback interrupt_callbacks_[32] = {nullptr};
    static inline bool interrupt_attached_[32] = {false};
    static inline bool eic_initialized_ = false;

    // Initialize the External Interrupt Controller
    static void InitEIC();
};

}