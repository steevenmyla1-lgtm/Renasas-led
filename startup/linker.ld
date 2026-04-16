/* linker.ld - RZT2L memory layout for J-Link RAM execution
 * Owned and qualified by project. No FSP dependency.
 * Device: R9A07G074M04 (RZT2L)
 * Reference: RZT2L Hardware User Manual
 */

/* RZT2L memory map */
ATCM_START   = 0x00000000;   /* CR52_0 ATCM - 512KB */
ATCM_SIZE    = 0x00080000;
BTCM_START   = 0x00100000;   /* CR52_0 BTCM - 64KB  */
BTCM_SIZE    = 0x00010000;
SYSRAM_START = 0x10000000;   /* System RAM   - 1MB  */
SYSRAM_SIZE  = 0x00100000;

/* Stack sizes per exception mode */
UND_STACK_SIZE = 0x00000100;
ABT_STACK_SIZE = 0x00000100;
FIQ_STACK_SIZE = 0x00000100;
IRQ_STACK_SIZE = 0x00000400;
SVC_STACK_SIZE = 0x00000400;

MEMORY
{
    ATCM   (rwx) : ORIGIN = ATCM_START,   LENGTH = ATCM_SIZE
    BTCM   (rwx) : ORIGIN = BTCM_START,   LENGTH = BTCM_SIZE
    SYSRAM (rw)  : ORIGIN = SYSRAM_START, LENGTH = SYSRAM_SIZE
}

ENTRY(reset_handler)

SECTIONS
{
    /* Exception vectors at ATCM base - CPU fetches reset vector from 0x00000000 */
    .vectors 0x00000000 : {
        KEEP(*(.vectors))
    } > ATCM

    /* Startup and application code */
    .text : {
        KEEP(*(.startup))
        *(.text*)
        *(.rodata*)
        . = ALIGN(4);
    } > ATCM

    /* Initialized data - loaded into ATCM, runs in ATCM (same for RAM execution) */
    _data_lma = LOADADDR(.data);
    .data : {
        _data_start = .;
        *(.data*)
        . = ALIGN(4);
        _data_end = .;
    } > ATCM

    /* Uninitialized data */
    .bss (NOLOAD) : {
        _bss_start = .;
        *(.bss*)
        *(COMMON)
        . = ALIGN(4);
        _bss_end = .;
    } > ATCM

    /* Exception mode stacks in BTCM (fast, separate from code/data)
     * Full-descending stack: SP starts at top (highest addr), grows down.
     * Label placed after space allocation = top address.
     */
    .stacks (NOLOAD) : {
        . = ALIGN(8);
        . += UND_STACK_SIZE;
        _und_stack_top = .;
        . += ABT_STACK_SIZE;
        _abt_stack_top = .;
        . += FIQ_STACK_SIZE;
        _fiq_stack_top = .;
        . += IRQ_STACK_SIZE;
        _irq_stack_top = .;
        . += SVC_STACK_SIZE;
        _svc_stack_top = .;
    } > BTCM
}
