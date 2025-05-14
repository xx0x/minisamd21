#pragma once

#include "minisamd21/System.hpp"
#include "Pin.hpp"
#include "samd21.h"

namespace minisamd21
{

/**
 * @brief Class for handling PWM outputs on the SAMD21 microcontroller.
 *
 * This class provides methods to initialize and control PWM outputs using the Timer Counter
 * Control (TCC) or Timer Counter (TC) peripherals on the SAMD21 microcontroller.
 *
 * The implementation automatically maps pins to their appropriate timer peripheral
 * based on the hardware capabilities of the SAMD21 chip.
 */
class PwmOutput
{
public:
    // Pin attribute definitions for PWM timer channels
    enum class TimerType
    {
        NONE,
        TC,
        TCC
    };

    // Structure to hold pin-to-timer mappings
    struct TimerMapping
    {
        Pin::PortName port;
        uint8_t pin;
        TimerType type;
        uint8_t instance;
        uint8_t channel;
        uint8_t mux_function; // Peripheral MUX function (usually 4 for TCC/TC)
    };

    /**
     * Constructor
     * @param pin The pin to use for PWM output
     */
    PwmOutput(Pin pin)
        : pin_(pin), frequency_(DEFAULT_FREQUENCY), duty_cycle_(0.0f),
          timer_channel_(0), timer_type_(TimerType::NONE), timer_instance_(nullptr)
    {
    }

    /**
     * Initialize the PWM output with the specified frequency
     * @param frequency PWM frequency in Hz (up to 24MHz)
     */
    void Init(uint32_t frequency = DEFAULT_FREQUENCY);

    /**
     * Set the duty cycle of the PWM output
     * @param duty_cycle Duty cycle as a float (0.0-1.0)
     */
    void Write(float duty_cycle);

    // Default frequency is 1kHz
    static constexpr uint32_t DEFAULT_FREQUENCY = 1000;

    // Maximum frequency depends on system clock
    static constexpr uint32_t MAX_FREQUENCY = System::FREQUENCY / 1000;

private:
    Pin pin_;            // Pin object
    uint32_t frequency_; // PWM frequency in Hz
    float duty_cycle_;   // Current duty cycle (0.0-1.0)

    // Internal variables for timer handling
    uint8_t timer_channel_; // TC or TCC channel number
    TimerType timer_type_;  // Type of timer (TC or TCC)
    void *timer_instance_;  // Pointer to TC or TCC instance

    // Flag array to track which timer instances are already enabled
#ifdef __SAMD21J18A__
    static inline bool timer_enabled_[7] = {false}; // TCC0, TCC1, TCC2, TC3, TC4, TC5, TC6
#else
    static inline bool timer_enabled_[6] = {false}; // TCC0, TCC1, TCC2, TC3, TC4, TC5
#endif

    // Map a pin to its timer channel
    void MapPinToTimer();

    // Synchronization helper functions
    inline void SyncTC(Tc *TCx);
    inline void SyncTCC(Tcc *TCCx);

    // Timer channel mapping table for SAMD21
    // Based on datasheet pinout tables
    // Format: {Port, Pin, Timer Type, Timer Instance, Timer Channel, MUX Function}
    // TODO: Double check all pins work!!!!
    static inline const TimerMapping TIMER_MAPPINGS[] = {
        // PORTA
        {Pin::PortName::PORTA, 4, TimerType::TCC, 0, 0, 4},  // PA04 = TCC0/WO[0] MUX E
        {Pin::PortName::PORTA, 5, TimerType::TCC, 0, 1, 4},  // PA05 = TCC0/WO[1] MUX E
        {Pin::PortName::PORTA, 6, TimerType::TCC, 1, 0, 4},  // PA06 = TCC1/WO[0] MUX E
        {Pin::PortName::PORTA, 7, TimerType::TCC, 1, 1, 4},  // PA07 = TCC1/WO[1] MUX E
        {Pin::PortName::PORTA, 8, TimerType::TCC, 0, 0, 5},  // PA08 = TCC0/WO[0] MUX F
        {Pin::PortName::PORTA, 9, TimerType::TCC, 0, 1, 5},  // PA09 = TCC0/WO[1] MUX F
        {Pin::PortName::PORTA, 10, TimerType::TCC, 1, 0, 5}, // PA10 = TCC1/WO[0] MUX F
        {Pin::PortName::PORTA, 11, TimerType::TCC, 1, 1, 5}, // PA11 = TCC1/WO[1] MUX F
        {Pin::PortName::PORTA, 12, TimerType::TCC, 2, 0, 5}, // PA12 = TCC2/WO[0] MUX F
        {Pin::PortName::PORTA, 13, TimerType::TCC, 2, 1, 5}, // PA13 = TCC2/WO[1] MUX F
        {Pin::PortName::PORTA, 15, TimerType::TC, 3, 1, 5},  // PA15 = TC3/WO[1] MUX F
        {Pin::PortName::PORTA, 16, TimerType::TCC, 2, 0, 5}, // PA16 = TCC2/WO[0] MUX F
        {Pin::PortName::PORTA, 17, TimerType::TCC, 2, 1, 5}, // PA17 = TCC2/WO[1] MUX F
        {Pin::PortName::PORTA, 18, TimerType::TC, 3, 0, 5},  // PA18 = TC3/WO[0] MUX F
        {Pin::PortName::PORTA, 19, TimerType::TC, 3, 1, 5},  // PA19 = TC3/WO[1] MUX F
#ifdef __SAMD21G18A__
        // These pins available on G18A but not on E18A
        {Pin::PortName::PORTA, 14, TimerType::TC, 3, 0, 5},  // PA14 = TC3/WO[0] MUX F
        {Pin::PortName::PORTA, 20, TimerType::TCC, 0, 6, 5}, // PA20 = TCC0/WO[6] MUX F
        {Pin::PortName::PORTA, 21, TimerType::TCC, 0, 7, 5}, // PA21 = TCC0/WO[7] MUX F
#endif
        {Pin::PortName::PORTA, 22, TimerType::TC, 4, 0, 5},  // PA22 = TC4/WO[0] MUX F
        {Pin::PortName::PORTA, 23, TimerType::TC, 4, 1, 5},  // PA23 = TC4/WO[1] MUX F

        // PORTB
        {Pin::PortName::PORTB, 8, TimerType::TC, 4, 0, 5},   // PB08 = TC4/WO[0] MUX F
        {Pin::PortName::PORTB, 9, TimerType::TC, 4, 1, 5},   // PB09 = TC4/WO[1] MUX F
        {Pin::PortName::PORTB, 10, TimerType::TC, 5, 0, 4},  // PB10 = TC5/WO[0] MUX E
        {Pin::PortName::PORTB, 11, TimerType::TC, 5, 1, 4},  // PB11 = TC5/WO[1] MUX E
#ifdef __SAMD21G18A__
        // These pins available on G18A but not on E18A
        {Pin::PortName::PORTB, 12, TimerType::TC, 4, 0, 4},  // PB12 = TC4/WO[0] MUX E
        {Pin::PortName::PORTB, 13, TimerType::TC, 4, 1, 4},  // PB13 = TC4/WO[1] MUX E
#endif
#ifdef __SAMD21J18A__
        // These pins available on J18A but not on E18A or G18A
        {Pin::PortName::PORTB, 16, TimerType::TC, 6, 0, 4},  // PB16 = TC6/WO[0] MUX E
        {Pin::PortName::PORTB, 17, TimerType::TC, 6, 1, 4},  // PB17 = TC6/WO[1] MUX E
#endif
        {Pin::PortName::PORTB, 30, TimerType::TCC, 0, 0, 4}, // PB30 = TCC0/WO[0] MUX E
        {Pin::PortName::PORTB, 31, TimerType::TCC, 0, 1, 4}, // PB31 = TCC0/WO[1] MUX E
    };
};

}