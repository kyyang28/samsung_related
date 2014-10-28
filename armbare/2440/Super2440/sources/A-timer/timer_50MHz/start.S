
.text
.global _start
_start:
    b    reset
    b    _undefined_instruction
    b    _software_interrupt
    b    _prefetch_abort
    b    _data_abort
    b    _reserved
    b    _irq
    b    _fiq

reset:
    /* Disable watchdog */
    mov  r0, #0
    ldr  r1, =0x53000000
    str  r0, [r1]
    
    ldr  sp, =4096

    bl   clock_init
    bl   sdram_init
    
    bl   copy_from_SRAM_to_SDRAM
    
    ldr  pc, =_sdram
    
_sdram:
    msr  cpsr_c, #0xD2    // IRQ mode
    ldr  sp, =4096
    
    msr  cpsr_c, #0xDF    // SYS mode
    ldr  sp, =0x34000000
    
    bl   led_init
    bl   timer0_init
    bl   irq_init
    
    ldr  lr, =_infinite_loop
    ldr  pc, =main
    
_infinite_loop:
    b    _infinite_loop


/* Exceptions  */
_undefined_instruction:
    b    _undefined_instruction

_software_interrupt:
    b    _software_interrupt

_prefetch_abort:
    b    _prefetch_abort

_data_abort:
    b    _data_abort

_reserved:
    b    _reserved

_irq:
    sub  lr, lr, #4
    stmdb sp!, {r0-r12, lr}

    ldr  lr, =int_return
    ldr  pc, =timer0_ISR

int_return:
    ldmia sp!, {r0-r12, pc}^

_fiq:
    b    _fiq


