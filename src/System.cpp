#include "minisamd21/System.hpp"
#include "samd21.h"

namespace minisamd21
{

void System::Init(ClockSource source)
{
    /* Set Flash wait states for 48 MHz */
    NVMCTRL->CTRLB.bit.RWS = 1; /* 1 wait state for 48MHz */

    // Set the clock source based on the provided parameter
    if (source == ClockSource::INTERNAL_OSC)
    {
        /* Enable OSC8M (internal 8MHz oscillator) with defaults */
        SYSCTRL->OSC8M.bit.PRESC = 0; /* No prescaler (divide by 1) */
        SYSCTRL->OSC8M.bit.ONDEMAND = 0;
        SYSCTRL->OSC8M.bit.RUNSTDBY = 0;
        SYSCTRL->OSC8M.bit.ENABLE = 1;
        while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_OSC8MRDY))
            ;

        /* Configure DFLL48M to use internal reference */
        SYSCTRL->DFLLCTRL.reg = 0; /* Make sure DFLL is disabled */
        while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY))
            ;

        /* Set DFLL coarse and fine values from NVM calibration values */
        uint32_t coarse = (*((uint32_t *)FUSES_DFLL48M_COARSE_CAL_ADDR) & FUSES_DFLL48M_COARSE_CAL_Msk) >> FUSES_DFLL48M_COARSE_CAL_Pos;
        SYSCTRL->DFLLVAL.reg = SYSCTRL_DFLLVAL_COARSE(coarse) | SYSCTRL_DFLLVAL_FINE(0x1ff);

        /* Configure DFLL in open-loop mode */
        SYSCTRL->DFLLCTRL.reg = SYSCTRL_DFLLCTRL_ENABLE;
        while (!(SYSCTRL->PCLKSR.reg & SYSCTRL_PCLKSR_DFLLRDY))
            ;

        /* Configure GCLK0 to use DFLL48M */
        GCLK->GENDIV.reg = GCLK_GENDIV_ID(0) | GCLK_GENDIV_DIV(1); /* No division */
        while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
            ;

        /* Configure GCLK0 to use DFLL48M and enable it */
        GCLK->GENCTRL.reg = GCLK_GENCTRL_ID(0) |
                            GCLK_GENCTRL_SRC_DFLL48M |
                            GCLK_GENCTRL_IDC |
                            GCLK_GENCTRL_GENEN;
        while (GCLK->STATUS.reg & GCLK_STATUS_SYNCBUSY)
            ;
    }
    else if (source == ClockSource::EXTERNAL_XTAL)
    {
        // TODO
    }
}

}