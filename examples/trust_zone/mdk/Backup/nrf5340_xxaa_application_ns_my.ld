/* Linker script to configure memory regions. */

SEARCH_DIR(.)
GROUP(-lgcc -lc -lnosys)

MEMORY
{
  FLASH (rx) : ORIGIN = 0x00050000, LENGTH = 0x40000 /*0x100000*/
  /*EXTFLASH (rx) : ORIGIN = 0x10000000, LENGTH = 0x8000000*/
  RAM (rwx) : ORIGIN = 0x20040000, LENGTH = 0x40000
  /*RAM (rwx) : ORIGIN = 0x20040000, LENGTH = 0x40000*/
  /*NON_SECURE_FUNC (rx) : ORIGIN = 0x00050000, LENGTH = 0x00000100*/ 

}


/* Linker script for Nordic Semiconductor nRF devices
 *
 * Version: Sourcery G++ 4.5-1
 * Support: https://support.codesourcery.com/GNUToolchain/
 *
 * Copyright (c) 2007, 2008, 2009, 2010 CodeSourcery, Inc.
 *
 * The authors hereby grant permission to use, copy, modify, distribute,
 * and license this software and its documentation for any purpose, provided
 * that existing copyright notices are retained in all copies and that this
 * notice is included verbatim in any distributions.  No written agreement,
 * license, or royalty fee is required for any of the authorized uses.
 * Modifications to this software may be copyrighted by their authors
 * and need not follow the licensing terms described here, provided that
 * the new terms are clearly indicated on the first page of each file where
 * they apply.
 */
OUTPUT_FORMAT ("elf32-littlearm", "elf32-bigarm", "elf32-littlearm")

/* Linker script to place sections and symbol values. Should be used together
 * with other linker script that defines memory regions FLASH and RAM.
 * It references following symbols, which must be defined in code:
 *   Reset_Handler : Entry of reset handler
 *
 * It defines following symbols, which code can use without definition:
 *   __exidx_start
 *   __exidx_end
 *   __etext
 *   __data_start__
 *   __preinit_array_start
 *   __preinit_array_end
 *   __init_array_start
 *   __init_array_end
 *   __fini_array_start
 *   __fini_array_end
 *   __data_end__
 *   __bss_start__
 *   __bss_end__
 *   __end__
 *   end
 *   __HeapBase
 *   __HeapLimit
 *   __StackLimit
 *   __StackTop
 *   __stack
 */
 /*nrf_common file mark*/
ENTRY(Reset_Handler)

__msp_init_stack_size__ = (0x00000400);

__psp_stack_size__ = (0x00000800);
/* NS_MSP_STACK_SIZE = (0x000000A0)*/

SECTIONS
{ 
    .text :
    {
        KEEP(*(.isr_vector))
        *(.text*)

        KEEP(*(.init))
        KEEP(*(.fini))

        /* .ctors */
        *crtbegin.o(.ctors)
        *crtbegin?.o(.ctors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .ctors)
        *(SORT(.ctors.*))
        *(.ctors)

        /* .dtors */
        *crtbegin.o(.dtors)
        *crtbegin?.o(.dtors)
        *(EXCLUDE_FILE(*crtend?.o *crtend.o) .dtors)
        *(SORT(.dtors.*))
        *(.dtors)

        *(.rodata*)

        KEEP(*(.eh_frame*))
    } > FLASH

     .TFM_DATA : ALIGN(4)
    {
        *(.data*)
        . = ALIGN(4);
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);
        . = ALIGN(4);
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);
        KEEP(*(.jcr*))
        . = ALIGN(4);
    } > RAM AT> FLASH
    Image$$ER_TFM_DATA$$RW$$Base = ADDR(.TFM_DATA);
    Image$$ER_TFM_DATA$$RW$$Limit = ADDR(.TFM_DATA) + SIZEOF(.TFM_DATA);

    .TFM_APP_ROT_LINKER_DATA : ALIGN(32)
    {
        *tfm_app_rot_partition*:*(.data*)
        *(TFM_*_APP-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_APP_ROT_LINKER_DATA$$RW$$Base = ADDR(.TFM_APP_ROT_LINKER_DATA);
    Image$$TFM_APP_ROT_LINKER_DATA$$RW$$Limit = ADDR(.TFM_APP_ROT_LINKER_DATA) + SIZEOF(.TFM_APP_ROT_LINKER_DATA);

    .TFM_PSA_ROT_LINKER_DATA : ALIGN(32)
    {
        *tfm_psa_rot_partition*:*(.data*)
        *(TFM_*_PSA-ROT_ATTR_RW)
        . = ALIGN(32);
    } > RAM AT> FLASH
    Image$$TFM_PSA_ROT_LINKER_DATA$$RW$$Base = ADDR(.TFM_PSA_ROT_LINKER_DATA);
    Image$$TFM_PSA_ROT_LINKER_DATA$$RW$$Limit = ADDR(.TFM_PSA_ROT_LINKER_DATA) + SIZEOF(.TFM_PSA_ROT_LINKER_DATA);


    .TFM_BSS : ALIGN(4)
    {
        __bss_start__ = .;
        __partition_runtime_start__ = .;
        KEEP(*(.bss.part_runtime))
        __partition_runtime_end__ = .;
        . = ALIGN(4);
        __service_runtime_start__ = .;
        KEEP(*(.bss.serv_runtime))
        __service_runtime_end__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM AT> RAM
    Image$$ER_TFM_DATA$$ZI$$Base = ADDR(.TFM_BSS);
    Image$$ER_TFM_DATA$$ZI$$Limit = ADDR(.TFM_BSS) + SIZEOF(.TFM_BSS);
    Image$$ER_PART_RT_POOL$$ZI$$Base = __partition_runtime_start__;
    Image$$ER_PART_RT_POOL$$ZI$$Limit = __partition_runtime_end__;
    Image$$ER_SERV_RT_POOL$$ZI$$Base = __service_runtime_start__;
    Image$$ER_SERV_RT_POOL$$ZI$$Limit = __service_runtime_end__;
    Image$$ER_TFM_DATA$$Base = ADDR(.TFM_DATA);
    Image$$ER_TFM_DATA$$Limit = ADDR(.TFM_DATA) + SIZEOF(.TFM_DATA) + SIZEOF(.TFM_BSS);
   
    .TFM_SECURE_STACK : ALIGN(128)
    {
        . += 0x2000;
    } > RAM
    Image$$TFM_SECURE_STACK$$ZI$$Base = ADDR(.TFM_SECURE_STACK);
    Image$$TFM_SECURE_STACK$$ZI$$Limit = ADDR(.TFM_SECURE_STACK) + SIZEOF(.TFM_SECURE_STACK);
    Image$$TFM_APP_RW_STACK_START$$Base = .;
   
    .TFM_PSA_ROT_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_PSA_ROT_LINKER = .;
        *tfm_psa_rot_partition*:*(.bss*)
        *tfm_psa_rot_partition*:*(COMMON)
        *(TFM_*_PSA-ROT_ATTR_ZI)
        . += (. - start_of_TFM_PSA_ROT_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_PSA_ROT_LINKER_DATA$$ZI$$Base = ADDR(.TFM_PSA_ROT_LINKER_BSS);
    Image$$TFM_PSA_ROT_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_PSA_ROT_LINKER_BSS) + SIZEOF(.TFM_PSA_ROT_LINKER_BSS);
    Image$$TFM_PSA_RW_STACK_END$$Base = .;
   
    .TFM_APP_ROT_LINKER_BSS : ALIGN(32)
    {
        start_of_TFM_APP_ROT_LINKER = .;
        *tfm_app_rot_partition*:*(.bss*)
        *tfm_app_rot_partition*:*(COMMON)
        *(TFM_*_APP-ROT_ATTR_ZI)
        . += (. - start_of_TFM_APP_ROT_LINKER) ? 0 : 4;
        . = ALIGN(32);
    } > RAM AT> RAM
    Image$$TFM_APP_ROT_LINKER_DATA$$ZI$$Base = ADDR(.TFM_APP_ROT_LINKER_BSS);
    Image$$TFM_APP_ROT_LINKER_DATA$$ZI$$Limit = ADDR(.TFM_APP_ROT_LINKER_BSS) + SIZEOF(.TFM_APP_ROT_LINKER_BSS);
    Image$$TFM_APP_RW_STACK_END$$Base = .;
    
    .copy.table : ALIGN(4)
    {
        __copy_table_start__ = .;
        LONG (LOADADDR(.TFM_DATA))  /*0x4 LONG 0xecf0 LOADADDR (.TFM_DATA*/
        LONG (ADDR(.TFM_DATA))      /*0x4 LONG 0x20000000 ADDR (.TFM_DATA)*/
        LONG (SIZEOF(.TFM_DATA))
        LONG (LOADADDR(.TFM_PSA_ROT_LINKER_DATA))
        LONG (ADDR(.TFM_PSA_ROT_LINKER_DATA))
        LONG (SIZEOF(.TFM_PSA_ROT_LINKER_DATA))
        LONG (LOADADDR(.TFM_APP_ROT_LINKER_DATA))
        LONG (ADDR(.TFM_APP_ROT_LINKER_DATA))
        LONG (SIZEOF(.TFM_APP_ROT_LINKER_DATA))
        __copy_table_end__ = .;
    } > FLASH

    .zero.table : ALIGN(4)
    {
        __zero_table_start__ = .;
        LONG (ADDR(.TFM_BSS))
        LONG (SIZEOF(.TFM_BSS))
        LONG (ADDR(.TFM_SECURE_STACK))
        LONG (SIZEOF(.TFM_SECURE_STACK))
        LONG (ADDR(.TFM_PSA_ROT_LINKER_BSS))
        LONG (SIZEOF(.TFM_PSA_ROT_LINKER_BSS))
        LONG (ADDR(.TFM_APP_ROT_LINKER_BSS))
        LONG (SIZEOF(.TFM_APP_ROT_LINKER_BSS))
        __zero_table_end__ = .;
    } > FLASH

    /*Secure gateways necessary for -cmse callaback functions*/
    .sg_start_dummy : ALIGN(4) 
    { 
        SG_START_DUMMY_UNUSED = .; 
    } > FLASH 
    
    .gnu.sgstubs (ALIGN((0x00004000)) - (0x400) + (. > (ALIGN((0x00004000)) - (0x400)) ? (0x00004000) : 0)) : 
    { 
        Load$$LR$$LR_VENEER$$Base = ABSOLUTE(.); 
        *(.gnu.sgstubs*) 
    } > FLASH 
    
    .sg_end_dummy : ALIGN(32) 
    { 
        . = (ALIGN((0x00004000))); 
        Load$$LR$$LR_VENEER$$Limit = ABSOLUTE(.); 
    } > FLASH ASSERT ((Load$$LR$$LR_VENEER$$Limit - Load$$LR$$LR_VENEER$$Base) <= (0x400), "Veneer region overflowed")

    .ARM.extab :
    {
        *(.ARM.extab* .gnu.linkonce.armextab.*)
    } > FLASH

    __exidx_start = .;
    .ARM.exidx :
    {
        *(.ARM.exidx* .gnu.linkonce.armexidx.*)
    } > FLASH
    __exidx_end = .;

    . = ALIGN(4);
    __etext = .;

    .data : AT (__etext)
    {
        __data_start__ = .;
        *(vtable)
        *(.data*)

        . = ALIGN(4);
        /* preinit data */
        PROVIDE_HIDDEN (__preinit_array_start = .);
        KEEP(*(.preinit_array))
        PROVIDE_HIDDEN (__preinit_array_end = .);

        . = ALIGN(4);
        /* init data */
        PROVIDE_HIDDEN (__init_array_start = .);
        KEEP(*(SORT(.init_array.*)))
        KEEP(*(.init_array))
        PROVIDE_HIDDEN (__init_array_end = .);


        . = ALIGN(4);
        /* finit data */
        PROVIDE_HIDDEN (__fini_array_start = .);
        KEEP(*(SORT(.fini_array.*)))
        KEEP(*(.fini_array))
        PROVIDE_HIDDEN (__fini_array_end = .);

        KEEP(*(.jcr*))
        . = ALIGN(4);
        /* All data end */
        __data_end__ = .;

    } > RAM

    .bss :
    {
        . = ALIGN(4);
        __bss_start__ = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        __bss_end__ = .;
    } > RAM

    .heap (COPY):
    {
        __HeapBase = .;
        __end__ = .;
        PROVIDE(end = .);
        KEEP(*(.heap*))
        __HeapLimit = .;
    } > RAM

    /* .stack_dummy section doesn't contains any symbols. It is only
     * used for linker to calculate size of stack sections, and assign
     * values to stack symbols later */
    .stack_dummy (COPY):
    {
        KEEP(*(.stack*))
    } > RAM


    /*
    .msp_stack : ALIGN(32)
    {
        . += __msp_init_stack_size__;
    } > RAM
    __StackTop = ADDR(.msp_stack);
    __StackLimit = ADDR(.msp_stack) + SIZEOF(.msp_stack);
    */

    .msp_stack : ALIGN(32)
    {
        . += __msp_init_stack_size__;
    } > RAM
    Image$$ARM_LIB_STACK$$ZI$$Base = ADDR(.msp_stack);
    Image$$ARM_LIB_STACK$$ZI$$Limit = ADDR(.msp_stack) + SIZEOF(.msp_stack);

    .psp_stack : ALIGN(32)
    {
        . += (__psp_stack_size__ - 0x8);
    } > RAM
    Image$$ER_INITIAL_PSP$$ZI$$Base = ADDR(.psp_stack);
    Image$$ER_INITIAL_PSP$$ZI$$Limit = ADDR(.psp_stack) + SIZEOF(.psp_stack);

    .psp_stack_seal_res :
    {
        . += 0x8;
    } > RAM
    Image$$ER_INITIAL_PSP_SEAL$$ZI$$Base = ADDR(.psp_stack_seal_res);
    Image$$ER_INITIAL_PSP_SEAL$$ZI$$Limit = ADDR(.psp_stack_seal_res) + SIZEOF(.psp_stack_seal_res);
   
    
    /* Set stack top to end of RAM, and stack limit move down by
     * size of stack_dummy section */
    __StackTop = ORIGIN(RAM) + LENGTH(RAM);
    __StackLimit = __StackTop - SIZEOF(.stack_dummy);

    /*Create a symbol for partition thing*/
    Load$$LR$$LR_NS_PARTITION$$Base = 0x00050000;
    PROVIDE(__stack = Image$$ER_INITIAL_PSP$$ZI$$Limit);

    /* Check if data + heap + stack exceeds RAM limit */
    ASSERT(__StackLimit >= __HeapLimit, "region RAM overflowed with stack")

    /* Check if text sections + data exceeds FLASH limit */
    DataInitFlashUsed = __bss_start__ - __data_start__;
    CodeFlashUsed = __etext - ORIGIN(FLASH);
    TotalFlashUsed = CodeFlashUsed + DataInitFlashUsed;
    ASSERT(TotalFlashUsed <= LENGTH(FLASH), "region FLASH overflowed with .data and user data")

}
