#include "minisamd21/I2C.hpp"

namespace minisamd21
{

void I2C::Init(Interface iface, uint32_t baud)
{
    // Set the interface
    if (iface == Interface::TWI0)
    {
        sercom_ = SERCOM0;
    }
    else if (iface == Interface::TWI1)
    {
        sercom_ = SERCOM1;
    }

    EnablePeripheral();

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

    // Send bus clear command if needed
    // if (!(sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(1)))
    // {
    //     // Force bus into idle state
    //     sercom_->I2CM.STATUS.reg = SERCOM_I2CM_STATUS_BUSSTATE(1);
    //     // Wait for change to take effect
    //     while (!(sercom_->I2CM.STATUS.reg & SERCOM_I2CM_STATUS_BUSSTATE(1)))
    //         ;
    // }

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

        // Read the data
        data[i] = sercom_->I2CM.DATA.reg;

        // If this is the last byte, send NACK before reading
        if (i == length - 1)
        {
            sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_ACKACT; // Send NACK
        }
        else
        {
            sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_ACKACT; // Send ACK
        }
    }

    // Send STOP condition
    sercom_->I2CM.CTRLB.reg &= ~SERCOM_I2CM_CTRLB_CMD_Msk; // Clear the CMD bits
    sercom_->I2CM.CTRLB.reg |= SERCOM_I2CM_CTRLB_CMD(0x3); // Set CMD to STOP
}

void I2C::WriteAddress(uint16_t address, uint8_t register_address, uint8_t address_size, bool nostop)
{
    if (address_size == 1)
    {
        Write(address, &register_address, 1, nostop);
    }
    else if (address_size == 2)
    {
        Write(address, (uint8_t *)&register_address, 2, nostop);
    }
}

void I2C::WriteRegisters(uint16_t address, uint8_t register_address, uint8_t address_size, uint8_t *data, uint32_t length, bool nostop)
{
    WriteAddress(address, register_address, address_size, nostop);
    Write(address, data, length, nostop);
}

void I2C::ReadRegisters(uint16_t address, uint8_t register_address, uint8_t address_size, uint8_t *data, uint32_t length, bool nostop)
{
    WriteAddress(address, register_address, address_size, nostop);
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
