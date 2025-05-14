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

    // Delay for specified milliseconds
    static void DelayMs(uint64_t delay_ms);
    
    // Delay for specified microseconds (approximately)
    static void DelayUs(uint32_t delay_us);

    // Get the number of milliseconds elapsed (tracked by System)
    static uint64_t GetMs();

    // Increment millisecond counter (called by ISR)
    static void Tick();

    // System clock frequency
    static constexpr uint32_t FREQUENCY = 48000000; // 48MHz

private:

    // Internal millisecond counter
    static inline uint64_t millis_ = 0;
};

}
