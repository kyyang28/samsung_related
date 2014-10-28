
.globl _start
_start:
	b	start_code
	ldr pc, _undefined_instruction
	ldr pc, _software_interrupt
	ldr pc, _prefetch_abort
	ldr pc, _data_abort
	ldr pc, _reserved
	ldr pc, _irq
	ldr pc, _fiq

_undefined_instruction:		.word undefined_instruction
_software_interrupt:		.word software_interrupt
_prefetch_abort:			.word prefetch_abort
_data_abort:				.word data_abort
_reserved:					.word reserved
_irq:						.word irq
_fiq:						.word fiq


start_code:

#define WTCON		0x53000000
	/* Disable watchdog */
	ldr	r0, =WTCON
	mov r1, #0x0
	str r1, [r0]

	/* Switch to IRQ mode and setup the irq's stack */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd2
	msr	cpsr, r0
	ldr sp, =2048

	/* Switch back to SVC mode and setup the svc's stack */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr cpsr, r0
	ldr sp, =4096

	/* Timer0 interrupt initialization */
	bl	timer0_irq_init

	ldr	lr, =infinite_loop

	/* Jump to timer0_irq_main */
	ldr pc, =timer0_irq_main

infinite_loop:
	b	infinite_loop

undefined_instruction:
	b	undefined_instruction

software_interrupt:
	b	software_interrupt

prefetch_abort:
	b	prefetch_abort

data_abort:
	b	data_abort

reserved:
	b	reserved

irq:
	sub lr, lr, #4

	stmdb sp!, {r0-r12,lr}
	ldr lr, =isr_ret
	ldr pc, =timer0_isr
isr_ret:
	ldmia sp!, {r0-r12,pc}^

fiq:
	b	fiq

