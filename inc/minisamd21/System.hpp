#pragma once

#include <cstdint>

namespace minisamd21
{

/**
 * @brief System class with common helper functions.
 */
class System
{
public:
    enum class ClockSource
    {
        INTERNAL_OSC, // Internal oscillator
        EXTERNAL_XTAL // External 32.768kHz crystal
    };

    // Function to initialize the system with selected oscillator source
    static void Init(ClockSource source);

    // Delay for specified milliseconds and update internal millisecond counter
    static void DelayMs(uint64_t delay);

    // Get the number of milliseconds elapsed (tracked by System)
    static uint64_t GetMs();

    // Increment millisecond counter (called by ISR)
    static void Tick();

private:
    // Internal millisecond counter
    static inline uint64_t millis_ = 0;
};

}
