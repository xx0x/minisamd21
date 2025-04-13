/* Define memory regions */
MEMORY
{
  FLASH (rx)  : ORIGIN = 0x00002000, LENGTH = 256K - 0x2000
  RAM   (rw)  : ORIGIN = 0x20000000, LENGTH = 32K
}

/* Define symbols for the memory areas */
_estack = ORIGIN(RAM) + LENGTH(RAM);

_etext = LOADADDR(.data);
_sdata = ADDR(.data);
_edata = _sdata + SIZEOF(.data);

_sbss = ADDR(.bss);
_ebss = _sbss + SIZEOF(.bss);

ENTRY(Reset_Handler)

SECTIONS {
  .vectors : {
    KEEP(*(.vectors))
    . = ALIGN(4);
  } > FLASH

  .text : {
    . = ALIGN(4);
    *(.text*)
    *(.rodata*)
    . = ALIGN(4);
  } > FLASH

  .data : AT(_etext) {
    . = ALIGN(4);
    *(.data*)
    . = ALIGN(4);
  } > RAM

  .bss : {
    . = ALIGN(4);
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
    /* Define _end symbol at the end of bss section */
    _end = .;
  } > RAM

  .stack (COPY): {
    . = ALIGN(8);
    stack_start = .;
    . = . + 0x1000;
    stack_end = .;
  } > RAM
  
  /* Remove information from standard libraries */
  /DISCARD/ : {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }
}
