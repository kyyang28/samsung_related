
.text
.global _start
_start:
    b    reset

_undefined_instruction:
    b    _undefined_instruction

    b    _swi

_prefetch_abort:
    b    _prefetch_abort

_data_abort:
    b    _data_abort

_reserved:
    b    _reserved

_irq:
    b    _irq

_fiq:
    b    _fiq

reset:
    /* Disable watchdog  */
    ldr  r0, =0x53000000
    mov  r1, #0
    str  r1, [r0]

    ldr  sp, =4096      // SVC mode

    msr  cpsr_c, #0xDF  // SYS mode
    ldr  sp, =3072

    msr  cpsr_c, #0xD0  // User mode, IRQ, FIQ disabled

    ldr  lr, =infinite_loop
    ldr  pc, =mymain

infinite_loop:
    b    infinite_loop


_swi:
    stmfd sp!, {r0-r12, lr}
    ldr   r4, [lr, #-4]
    bic   r4, r4, #0xFF000000

    cmp   r4, #1
    ldreq lr, =swi_return
    ldreq pc, =sys_led_on

    cmp   r4, #2
    ldreq lr, =swi_return
    ldreq pc, =sys_led_off

    cmp   r4, #3
    ldreq lr, =swi_return
    ldreq pc, =sys_led_all_on

    cmp   r4, #4
    ldreq lr, =swi_return
    ldreq pc, =sys_led_all_off

    movne r0, #-1

swi_return:
    ldmia sp!, {r0-r12, pc}^

