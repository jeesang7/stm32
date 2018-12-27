/*
*****************************************************************************
**

**  File        : LinkerScript.ld
**
**  Abstract    : Linker script for STM32F769NIHx Device with
**                2048KByte FLASH, 512KByte RAM
**
**                Set heap size, stack size and stack location according
**                to application requirements.
**
**                Set memory bank area and size if external memory is used.
**
**  Target      : STMicroelectronics STM32
**
**
**  Distribution: The file is distributed as is, without any warranty
**                of any kind.
**
**  (c)Copyright Ac6.
**  You may use this file as-is or modify it according to the needs of your
**  project. Distribution of this file (unmodified or modified) is not
**  permitted. Ac6 permit registered System Workbench for MCU users the
**  rights to distribute the assembled, compiled & linked contents of this
**  file as part of an application binary file, provided that it is built
**  using the System Workbench for MCU toolchain.
**
*****************************************************************************
*/

/* Entry Point */
ENTRY(Reset_Handler)
__ICFEDIT_intvec_start__  = 0x08000000;


/* Highest address of the user mode stack */
_estack = 0x2007C000;    /* end of RAM */
/* Generate a link error if heap and stack don't fit into RAM */
_Min_Heap_Size = 0x17800; /* required amount of heap  */
_Min_Stack_Size = 0x2000; /* required amount of stack */

/* Specify the memory areas */
/* SRAM2 : [0x2007C000, 0x2007FFFF] : 16 KBytes                                    */
/*         [0x2007C000,0x2007C1FF]  : 512 Bytes     : Ethernet dma descripteurs    */
/*         [0x2007C200,0x2007FFFF]  : 15.5 KBytes   : Ethernet Buffers             */
MEMORY
{
RAM (xrw)       : ORIGIN = 0x20000000, LENGTH = 496K
FLASH (rx)      : ORIGIN = 0x08000000, LENGTH = 32K
UFIXED_LOC (rx) : ORIGIN = 0x08008000, LENGTH = 32K 
ROM2 (rx)       : ORIGIN = 0x08010000, LENGTH = 1984K
RAM_NO_CACHE (xrw)             : ORIGIN = 0x2007C000, LENGTH = 16K
RAM_NO_CACHE_DEVICE (xrw)      : ORIGIN = 0x2007C000, LENGTH = 512
RAM_NO_CACHE_NORMAL (xrw)      : ORIGIN = 0x2007C200, LENGTH = 15872
}

/* Define output sections */
SECTIONS
{
  /* The startup code goes first into FLASH */
  .isr_vector :
  {
    . = ALIGN(4);
    KEEP(*(.isr_vector)) /* Startup code */
    . = ALIGN(4);
  } >FLASH

  /* The program code and other data goes into FLASH */
  .text :
  {
    . = ALIGN(4);
    *(.text)           /* .text sections (code) */
    *(.text*)          /* .text* sections (code) */
    *(.glue_7)         /* glue arm to thumb code */
    *(.glue_7t)        /* glue thumb to arm code */
    *(.eh_frame)

    KEEP (*(.init))
    KEEP (*(.fini))

    . = ALIGN(4);
    _etext = .;        /* define a global symbols at end of code */
  } >ROM2

  /* Constant data goes into FLASH */
  .rodata :
  {
    . = ALIGN(4);
    *(.rodata)         /* .rodata sections (constants, strings, etc.) */
    *(.rodata*)        /* .rodata* sections (constants, strings, etc.) */
    . = ALIGN(4);
  } >ROM2

  .ARM.extab   : { *(.ARM.extab* .gnu.linkonce.armextab.*) } >FLASH
  .ARM : {
    __exidx_start = .;
    *(.ARM.exidx*)
    __exidx_end = .;
  } >FLASH

  .preinit_array     :
  {
    PROVIDE_HIDDEN (__preinit_array_start = .);
    KEEP (*(.preinit_array*))
    PROVIDE_HIDDEN (__preinit_array_end = .);
  } >FLASH
  .init_array :
  {
    PROVIDE_HIDDEN (__init_array_start = .);
    KEEP (*(SORT(.init_array.*)))
    KEEP (*(.init_array*))
    PROVIDE_HIDDEN (__init_array_end = .);
  } >FLASH
  .fini_array :
  {
    PROVIDE_HIDDEN (__fini_array_start = .);
    KEEP (*(SORT(.fini_array.*)))
    KEEP (*(.fini_array*))
    PROVIDE_HIDDEN (__fini_array_end = .);
  } >FLASH

  /* used by the startup to initialize data */
  _sidata = LOADADDR(.data);

  /* Initialized data sections goes into RAM, load LMA copy after code */
  .data : 
  {
    . = ALIGN(4);
    _sdata = .;        /* create a global symbol at data start */
    *(.data)           /* .data sections */
    *(.data*)          /* .data* sections */

    . = ALIGN(4);
    _edata = .;        /* define a global symbol at data end */
  } >RAM AT> FLASH

  
  /* Uninitialized data section */
  . = ALIGN(4);
  .bss :
  {
    /* This is used by the startup in order to initialize the .bss secion */
    _sbss = .;         /* define a global symbol at bss start */
    __bss_start__ = _sbss;
    *(.bss)
    *(.bss*)
    *(COMMON)

    . = ALIGN(4);
    _ebss = .;         /* define a global symbol at bss end */
    __bss_end__ = _ebss;
  } >RAM

  /* User_heap_stack section, used to check that there is enough RAM left */
  ._user_heap_stack :
  {
    . = ALIGN(8);
    PROVIDE ( _user_heap_base = . );
    . = . + _Min_Heap_Size;
    PROVIDE ( _user_heap_end = . );
    PROVIDE ( _user_stack_base = . );
    . = . + _Min_Stack_Size;
    PROVIDE ( _user_stack_end = . );
    . = ALIGN(8);
  } >RAM

  

  /* Remove information from the standard libraries */
  /DISCARD/ :
  {
    libc.a ( * )
    libm.a ( * )
    libgcc.a ( * )
  }

  .ARM.attributes 0 : { *(.ARM.attributes) }
  .UNINIT_FIXED_LOC (NOLOAD): { *(UNINIT_FIXED_LOC) } >UFIXED_LOC
  .sram2_non_cached_device : { *(.sram2_non_cached_device) } >RAM_NO_CACHE_DEVICE AT> ROM2
  .sram2_non_cached_normal : { *(.sram2_non_cached_normal) } >RAM_NO_CACHE_NORMAL AT> ROM2
}


