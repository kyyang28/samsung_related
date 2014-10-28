
.equ    rWTCON,    0x53000000

.text
.global _start
_start:
	/* 0x0 */
    b    reset

    /* 0x4 */
    b    _undefined_instruction	

    /* 0x8 */
    b    _swi

    /* 0xC */
    b    _prefetch_abort 

    /* 0x10 */
    b    _data_abort 

    /* 0x14 */
    b    _reserved 

    /* 0x18 */
    b    _irq  

    /* 0x1C */
    b    _fiq  


reset:
    /* Disable watchdog */
    mov r0, #0
    ldr r1, =rWTCON
    str r0, [r1]

    ldr sp, =4096

    bl clock_init
    bl sdram_init
    bl nand_init

    ldr r0, =0x30000000
    mov r1, #0
    ldr r2, =__bss_start
    sub r2, r2, r0
    bl  CopyCode2RAM

    bl  clear_bss

clear_bss:
    ldr r0, =__bss_start
    ldr r1, =__bss_end
    mov r3, #0
    cmp r0, r1
    b   mode_init
clr_loop:
    str r3, [r0], #4
    cmp r0, r1
    bne clr_loop

mode_init:
    msr cpsr_c, #0xD2 // IRQ mode
    ldr sp, =0x31000000

    msr cpsr_c, #0xDF
    
    ldr sp, =0x34000000

    ldr lr, =irq_init_return
    ldr pc, =irq_init

irq_init_return:
	
    ldr lr, =uart_init_return
    mov r0, #0                   @ first param
    ldr pc, =uart_init

uart_init_return:
	
    ldr lr, =IIC_init_return
    ldr pc, =IIC_init

IIC_init_return:

    ldr lr, =infinite_loop
    ldr pc, =main

infinite_loop:
    b   infinite_loop


_undefined_instruction:
    b    _undefined_instruction

_swi:
    b    _swi

_prefetch_abort:
    b    _prefetch_abort

_data_abort:
    b    _data_abort

_reserved:
    b    _reserved

_irq:
    sub   lr, lr, #4
    stmdb sp!, {r0-r12, lr}

    ldr   lr, =_int_return
    ldr   pc, =IRQ_ISR_ENTRY

_int_return:
    ldmia sp!, {r0-r12, pc}^

_fiq:
    b    _fiq


