#include "minisamd21/I2C.hpp"

namespace minisamd21
{

I2C::I2C(Interface iface)
{
    // Set the interface
    switch (iface)
    {
    case Interface::TWI0:
        sercom_ = SERCOM0;
        break;
    case Interface::TWI1:
        sercom_ = SERCOM1;
        break;
    default:
        sercom_ = nullptr;
        break;
    }
    EnablePeripheral();
}

void I2C::Init(uint32_t baud)
{
    // Init
    sercom_->I2CM.CTRLA.reg = SERCOM_I2CM_CTRLA_MODE(0x5) |
                              SERCOM_I2CM_CTRLA_SDAHOLD(0x3) |
                              SERCOM_I2CM_CTRLA_SPEED(0x1);

    sercom_->I2CM.BAUD.reg = (uint16_t)((48000000 / (2 * baud)) - 1);

    sercom_->I2CM.CTRLA.reg |= SERCOM_I2CM_CTRLA_ENABLE;
    while (sercom_->I2CM.SYNCBUSY.reg)
    {
    }
}

void I2C::DeInit()
{
    // Disable the I2C interface
    sercom_->I2CM.CTRLA.reg &= ~SERCOM_I2CM_CTRLA_ENABLE;
    while (sercom_->I2CM.SYNCBUSY.reg)
    {
    }

    // Reset the SERCOM module
    sercom_->I2CM.CTRLA.bit.SWRST = 1;
    while (sercom_->I2CM.CTRLA.bit.SWRST || sercom_->I2CM.SYNCBUSY.bit.SWRST)
    {
    }
}

void I2C::Write(uint8_t address, uint8_t *data, uint32_t length, bool nostop)
{
    // First, check bus status and clear any error conditions
    if (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSERR)
    {
        // Clear bus error flag
        sercom_->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSERR;
    }

    // Send bus clear command if needed
    if (!(sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(1)))
    {
        // Force bus into idle state
        sercom_->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);
        // Wait for change to take effect
        while (!(sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(1)))
            ;
    }

    // Send repeated start command
    sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk; // Clear the CMD bits
    sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x1); // Set CMD to START

    // Write address - shifted left by 1 and LSB set to 0 for write
    sercom_->I2CM.ADDR.reg = (address << 1) & ~0x01;

    // Wait for acknowledgment (ACK) or detect NACK
    while (!(sercom_->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB))
    {
        if (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
        {
            // Send STOP condition
            sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x3);
            return; // Exit if no acknowledgment
        }
    }

    for (uint32_t i = 0; i < length; ++i)
    {
        sercom_->I2CM.DATA.reg = data[i];

        // Wait for MB flag with timeout
        uint32_t timeout = 100000;
        while (!(sercom_->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_MB) && timeout--)
        {
            // Exit if we get NACK or bus error
            if (sercom_->I2CM.STATUS.reg & (SERCOM_I2CM_STATUS_RXNACK | SERCOM_I2CM_STATUS_BUSERR))
            {
                break;
            }
        }

        // Check for NACK
        if (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
        {
            break; // Exit loop on NACK
        }
    }

    if (!nostop)
    {
        // Send STOP condition
        sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk; // Clear the CMD bits
        sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x3); // Set CMD to STOP
    }
}

void I2C::Read(uint8_t address, uint8_t *data, uint32_t length)
{
    // First, check bus status and clear any error conditions
    if (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSERR)
    {
        // Clear bus error flag
        sercom_->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSERR;
    }

    // Always start with ACK enabled (we want to ACK all but the last byte)
    sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT;

    // Send repeated start command
    sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk; // Clear the CMD bits
    sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x1); // Set CMD to START

    // Write address - shifted left by 1 and LSB set to 1 for read
    sercom_->I2CM.ADDR.reg = ((address << 1) | 0x01);

    // Check for NACK (no acknowledgment)
    if (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_RXNACK)
    {
        // Send STOP condition
        sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk;
        sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x3);
        return; // Exit if no acknowledgment
    }

    for (uint32_t i = 0; i < length; ++i)
    {
        // Wait for SB (Slave on Bus) flag with timeout
        uint32_t timeout = 100000;
        while (!(sercom_->I2CM.INTFLAG.reg & SERCOM_I2CM_INTFLAG_SB) && timeout--)
        {
            // Exit if we get bus error
            if (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSERR)
            {
                break;
            }
        }

        // Check if timed out or encountered an error
        if (timeout == 0 || (sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSERR))
        {
            break; // Exit loop on timeout or error
        }

        // Before reading the last byte, set NACK to indicate end of transfer
        if (i == length - 1)
        {
            // For the last byte, we've already read it, so just break the loop
            data[i] = sercom_->I2CM.DATA.reg;
            // Set NACK for the last byte
            sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT;
            break;
        }

        // Read current byte
        data[i] = sercom_->I2CM.DATA.reg;

        // Issue READ command to continue transaction
        sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk;
        sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x2); // Set CMD to READ
    }

    // Send STOP condition
    sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk; // Clear the CMD bits
    sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x3); // Set CMD to STOP
}

void I2C::FillAddress(uint16_t register_address, uint8_t register_address_size, uint8_t *data)
{
    if (register_address_size == 1)
    {
        data[0] = register_address;
    }
    else if (register_address_size == 2)
    {
        data[0] = (register_address >> 8) & 0xFF; // MSB
        data[1] = register_address & 0xFF;        // LSB
    }
}

void I2C::WriteRegisters(uint16_t address, uint16_t register_address, uint8_t register_address_size, uint8_t *data, uint32_t length)
{
    uint8_t combined_data[length + register_address_size] = {0};
    FillAddress(register_address, register_address_size, combined_data);
    for (uint32_t i = 0; i < length; ++i)
    {
        combined_data[i + register_address_size] = data[i];
    }
    Write(address, combined_data, length + 1);
}

void I2C::ReadRegisters(uint16_t address, uint16_t register_address, uint8_t register_address_size, uint8_t *data, uint32_t length)
{
    uint8_t address_data[register_address_size] = {0};
    FillAddress(register_address, register_address_size, address_data);
    Write(address, address_data, register_address_size, true);
    //WriteAddress(address, register_address, address_size, true);
    Read(address, data, length);
}

void I2C::EnablePeripheral()
{
    if (sercom_ == SERCOM0)
    {
        // Enable SERCOM0 clock
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM0;

        // Set up GCLK for SERCOM0
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM0_CORE_Val) |
                            GCLK_CLKCTRL_GEN_GCLK0 |
                            GCLK_CLKCTRL_CLKEN;

        // Wait for synchronization
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
        }

        // Configure pins
        PORT->Group[0].PINCFG[8].reg |= PORT_PINCFG_PMUXEN;
        PORT->Group[0].PINCFG[9].reg |= PORT_PINCFG_PMUXEN;
        PORT->Group[0].PMUX[4].reg |= PORT_PMUX_PMUXE_C;
        PORT->Group[0].PMUX[4].reg |= PORT_PMUX_PMUXO_C;
    }
    else if (sercom_ == SERCOM1)
    {
        // Enable SERCOM1 clock
        PM->APBCMASK.reg |= PM_APBCMASK_SERCOM1;

        // Set up GCLK for SERCOM1
        GCLK->CLKCTRL.reg = GCLK_CLKCTRL_ID(GCLK_CLKCTRL_ID_SERCOM1_CORE_Val) |
                            GCLK_CLKCTRL_GEN_GCLK0 |
                            GCLK_CLKCTRL_CLKEN;

        // Wait for synchronization
        while (GCLK->STATUS.bit.SYNCBUSY)
        {
        }

        // Configure pins
        PORT->Group[0].PINCFG[22].reg |= PORT_PINCFG_PMUXEN;
        PORT->Group[0].PINCFG[23].reg |= PORT_PINCFG_PMUXEN;
        PORT->Group[0].PMUX[11].reg |= PORT_PMUX_PMUXE_C;
        PORT->Group[0].PMUX[11].reg |= PORT_PMUX_PMUXO_C;
    }

    // Reset the SERCOM module before configuration
    sercom_->I2CM.CTRLA.bit.SWRST = 1;
    while (sercom_->I2CM.CTRLA.bit.SWRST || sercom_->I2CM.SYNCBUSY.bit.SWRST)
    {
    }
}

} // namespace minisamd21
