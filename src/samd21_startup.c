#include <stdint.h>
#include <sys/stat.h>
#include <errno.h>
#include <unistd.h>
#include "samd21.h"

extern int main();
extern uint32_t _etext, _sdata, _edata, _sbss, _ebss, _estack;
extern uint32_t _end; /* Define _end symbol for heap management */

/* Forward declaration of exception handlers */
void Reset_Handler(void);
void NMI_Handler(void) __attribute__((weak, alias("Default_Handler")));
void HardFault_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SVC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PendSV_Handler(void) __attribute__((weak, alias("Default_Handler")));
extern void SysTick_Handler(void); // Handled in System.cpp

/* SAMD21 peripheral interrupt handlers */
void PM_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SYSCTRL_Handler(void) __attribute__((weak, alias("Default_Handler")));
void WDT_Handler(void) __attribute__((weak, alias("Default_Handler")));
void RTC_Handler(void) __attribute__((weak, alias("Default_Handler")));
extern void EIC_Handler(void); // Handled in Pin.cpp
void NVMCTRL_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DMAC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void USB_Handler(void) __attribute__((weak, alias("Default_Handler")));
void EVSYS_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void SERCOM5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC0_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC1_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TCC2_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC3_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC4_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC5_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC6_Handler(void) __attribute__((weak, alias("Default_Handler")));
void TC7_Handler(void) __attribute__((weak, alias("Default_Handler")));
void ADC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void AC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void DAC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void PTC_Handler(void) __attribute__((weak, alias("Default_Handler")));
void I2S_Handler(void) __attribute__((weak, alias("Default_Handler")));


void Reset_Handler(void) {
    /* Copy initialized data from flash to RAM */
    uint32_t* src = &_etext;
    uint32_t* dst = &_sdata;
    while (dst < &_edata) {
        *dst++ = *src++;
    }

    /* Clear the BSS section */
    dst = &_sbss;
    while (dst < &_ebss) {
        *dst++ = 0;
    }
    
   // Clock needs to be initialized inside main()
    
    /* Call main */
    main();
    
    /* If main returns, loop forever */
    while (1) {}
}

void Default_Handler(void) {
    while (1) {}
}

/* Align vector table to 256-byte boundary as required by ARM */
__attribute__((section(".vectors"), aligned(256)))
const void* vector_table[] = {
    (void*)&_estack,                /* Initial Stack Pointer */
    (void*)Reset_Handler,           /* Reset Handler */
    (void*)NMI_Handler,             /* NMI Handler */
    (void*)HardFault_Handler,       /* Hard Fault Handler */
    0,                              /* Reserved */
    0,                              /* Reserved */
    0,                              /* Reserved */
    0,                              /* Reserved */
    0,                              /* Reserved */
    0,                              /* Reserved */
    0,                              /* Reserved */
    (void*)SVC_Handler,             /* SVCall Handler */
    0,                              /* Reserved */
    0,                              /* Reserved */
    (void*)PendSV_Handler,          /* PendSV Handler */
    (void*)SysTick_Handler,         /* SysTick Handler */
    
    /* External Interrupts */
    (void*)PM_Handler,              /* Power Manager */
    (void*)SYSCTRL_Handler,         /* System Control */
    (void*)WDT_Handler,             /* Watchdog Timer */
    (void*)RTC_Handler,             /* Real-Time Counter */
    (void*)EIC_Handler,             /* External Interrupt Controller */
    (void*)NVMCTRL_Handler,         /* Non-Volatile Memory Controller */
    (void*)DMAC_Handler,            /* Direct Memory Access Controller */
    (void*)USB_Handler,             /* Universal Serial Bus */
    (void*)EVSYS_Handler,           /* Event System Interface */
    (void*)SERCOM0_Handler,         /* Serial Communication Interface 0 */
    (void*)SERCOM1_Handler,         /* Serial Communication Interface 1 */
    (void*)SERCOM2_Handler,         /* Serial Communication Interface 2 */
    (void*)SERCOM3_Handler,         /* Serial Communication Interface 3 */
    (void*)SERCOM4_Handler,         /* Serial Communication Interface 4 */
    (void*)SERCOM5_Handler,         /* Serial Communication Interface 5 */
    (void*)TCC0_Handler,            /* Timer Counter Control 0 */
    (void*)TCC1_Handler,            /* Timer Counter Control 1 */
    (void*)TCC2_Handler,            /* Timer Counter Control 2 */
    (void*)TC3_Handler,             /* Basic Timer Counter 3 */
    (void*)TC4_Handler,             /* Basic Timer Counter 4 */
    (void*)TC5_Handler,             /* Basic Timer Counter 5 */
    (void*)TC6_Handler,             /* Basic Timer Counter 6 */
    (void*)TC7_Handler,             /* Basic Timer Counter 7 */
    (void*)ADC_Handler,             /* Analog Digital Converter */
    (void*)AC_Handler,              /* Analog Comparators */
    (void*)DAC_Handler,             /* Digital Analog Converter */
    (void*)PTC_Handler,             /* Peripheral Touch Controller */
    (void*)I2S_Handler              /* Inter-IC Sound Interface */
};

caddr_t _sbrk(int incr) {
    static unsigned char *heap = NULL;
    unsigned char *prev_heap;
    if (heap == NULL) {
        heap = (unsigned char *)&_end;
    }
    prev_heap = heap;
    heap += incr;
    return (caddr_t)prev_heap;
}

int _kill(int pid, int sig) {
    errno = EINVAL;
    return -1;
}

int _getpid(void) {
    return 1;
}

void _exit(int status) {
    while (1) {}
}
