#pragma once
#include "Pin.hpp"
#include "samd21.h"

namespace minisamd21
{

/**
 * @brief For now it supports blocking mode reading only.
 */
class AdcInput
{
public:
    // Reference voltage options
    enum class Reference
    {
        INTVCC1, // 1/2 VDDANA (default; allows reading the full 0 to VDDANA range because of the gain divider)
        INTVCC0, // 1/1.48 VDDANA
        INT1V,   // Internal 1.0V reference
        AREF,    // External reference on AREF pin
    };

    // Resolution options
    enum class Resolution
    {
        BIT12, // 12-bit resolution
        BIT10, // 10-bit resolution
        BIT8,  // 8-bit resolution
    };

    // Number of samples to average
    // As defined by ADC_AVGCTRL_SAMPLENUM_*_Val
    enum class Averaging
    {
        SAMPLES_1,
        SAMPLES_2,
        SAMPLES_4,
        SAMPLES_8,
        SAMPLES_16,
        SAMPLES_32,
        SAMPLES_64,
        SAMPLES_128,
        SAMPLES_256,
        SAMPLES_512,
        SAMPLES_1024
    };

    // Constructor
    AdcInput(Pin pin);

    // Initialize the ADC
    void Init(
        Resolution res = Resolution::BIT12,
        Reference ref = Reference::INTVCC1);

    // Read the ADC value
    uint16_t Read() const;

    // Set the reference voltage
    void SetReference(Reference ref);

    // Set the resolution
    void SetResolution(Resolution res);

    // Set the number of samples to average
    void SetAveraging(Averaging samples);

private:
    Pin pin_;         // Pin object
    uint8_t channel_; // ADC input channel number

    // Map pin to ADC channel
    static uint8_t MapPinToChannel(Pin pin);

    inline void SyncBusy() const
    {
        while (ADC->STATUS.bit.SYNCBUSY)
            ;
    }
};

}
