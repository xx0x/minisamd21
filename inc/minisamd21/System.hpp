#pragma once

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
};

}
