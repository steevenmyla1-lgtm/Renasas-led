/* startup.S - Cortex-R52 minimal startup for RZT2L
 * Owned and qualified by project. No FSP dependency.
 */

.syntax unified
.arm

/* ARM processor mode encodings */
.equ MODE_UND, 0x1B
.equ MODE_ABT, 0x17
.equ MODE_FIQ, 0x11
.equ MODE_IRQ, 0x12
.equ MODE_SVC, 0x13
.equ MASK_IRQ, 0x80
.equ MASK_FIQ, 0x40

/* Exception vector table - must be placed at ATCM base (0x00000000) */
.section .vectors, "ax"
.global _vectors
_vectors:
    b   reset_handler
    b   undef_handler
    b   svc_handler
    b   prefetch_abort_handler
    b   data_abort_handler
    b   .                          /* reserved */
    b   irq_handler
    b   fiq_handler

/* Reset handler */
.section .startup, "ax"
.global reset_handler
reset_handler:
    /* Disable IRQ and FIQ interrupts */
    cpsid   if

    /* Set up stacks for each exception mode */
    cps     #MODE_UND
    ldr     sp, =_und_stack_top

    cps     #MODE_ABT
    ldr     sp, =_abt_stack_top

    cps     #MODE_FIQ
    ldr     sp, =_fiq_stack_top

    cps     #MODE_IRQ
    ldr     sp, =_irq_stack_top

    /* Return to SVC mode (default after reset) */
    cps     #MODE_SVC
    ldr     sp, =_svc_stack_top

    /* Copy .data section from load address (ROM/ATCM) to run address */
    ldr     r0, =_data_lma
    ldr     r1, =_data_start
    ldr     r2, =_data_end
    b       .Ldata_check
.Ldata_copy:
    ldr     r3, [r0], #4
    str     r3, [r1], #4
.Ldata_check:
    cmp     r1, r2
    blt     .Ldata_copy

    /* Zero .bss section */
    ldr     r1, =_bss_start
    ldr     r2, =_bss_end
    mov     r3, #0
    b       .Lbss_check
.Lbss_zero:
    str     r3, [r1], #4
.Lbss_check:
    cmp     r1, r2
    blt     .Lbss_zero

    /* Call main */
    bl      main

    /* main() should never return - trap here */
.Lhalt:
    b       .Lhalt

/* Default exception handlers - trap (replace with real handlers as needed) */
.weak undef_handler
undef_handler:
    b   undef_handler

.weak svc_handler
svc_handler:
    b   svc_handler

.weak prefetch_abort_handler
prefetch_abort_handler:
    b   prefetch_abort_handler

.weak data_abort_handler
data_abort_handler:
    b   data_abort_handler

.weak irq_handler
irq_handler:
    b   irq_handler

.weak fiq_handler
fiq_handler:
    b   fiq_handler
