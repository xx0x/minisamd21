#include <stdint.h>

#define LED_PORT 0
#define LED_PIN  23

#include "samd21.h"

void delay_ms(uint32_t ms) {
    // For 48MHz clock (48,000 cycles per millisecond)
    SysTick->LOAD  = 48000 - 1;
    SysTick->VAL   = 0;
    SysTick->CTRL  = SysTick_CTRL_CLKSOURCE_Msk | SysTick_CTRL_ENABLE_Msk;

    for (uint32_t i = 0; i < ms; ++i) {
        while ((SysTick->CTRL & SysTick_CTRL_COUNTFLAG_Msk) == 0);
    }

    SysTick->CTRL = 0;
}

void init_led() {
    /* Enable PORT peripheral clock */
    PM->APBBMASK.reg |= PM_APBBMASK_PORT;
    
    /* Configure LED pin as output */
    PORT->Group[LED_PORT].DIRSET.reg = (1 << LED_PIN);
    
    /* Set initial state to off (low) */
    PORT->Group[LED_PORT].OUTCLR.reg = (1 << LED_PIN);
}

void toggle_led() {
    PORT->Group[LED_PORT].OUTTGL.reg = (1 << LED_PIN);
}

int main() {
    /* Initialize the LED pin */
    init_led();
    
    /* Main program loop */
    while (1) {
        toggle_led();
        delay_ms(500);
    }
}
