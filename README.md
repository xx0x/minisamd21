# minisamd21

Minimalist library for making projects based on SAMD21.

The link address is 0x00002000 to work with Adafruit UF2 bootloader.

**In progress, not ready for production stuff.**

## Features

| Name                           | State                |
| ------------------------------ | -------------------- |
| Pins (write, read, interrupts) | ✅                    |
| ADC                            | ✅ (only basic read)  |
| PWM                            | ✅                    |
| I2C                            | ✅ (only synchronous) |
| Sleep                          | ✅                    |
| SPI                            | 🚧                    |
| DAC                            | 🚧                    |
| I2S                            | 🚧                    |
| RTC (internal)                 | 🚧                    |

## Libraries for devices

| Device           | Description        | State |
| ---------------- | ------------------ | ----- |
| DS3231           | Real time clock    | ✅     |
| AT24xx           | Serial EEPROM      | ✅     |
| OutShiftRegister | 74HC595 and others | ✅     |



