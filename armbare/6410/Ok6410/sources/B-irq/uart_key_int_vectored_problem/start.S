
.globl _start
_start:	
	b	start_code
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _not_used
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:	.word undefined_instruction
_software_interrupt:	.word software_interrupt
_prefetch_abort:		.word prefetch_abort
_data_abort:			.word data_abort
_not_used:				.word not_used
_irq:					.word irq
_fiq:					.word fiq


start_code:
	/* Peripheral port setup */
	ldr r0, =0x70000000
	orr r0, r0, #0x13
	mcr p15,0,r0,c15,c2,4
	
	/*	
	 *	OPTIONS: During BL0, the watchdog is already disabled, 
	 *	so actually we don't need to disable it again(in BL1).
	 */
	ldr	r0, =0x7E004000
	mov	r1, #0
	str r1, [r0]
	
	/*	
	 *	WARNING: When booting from SD card, DO NOT setup the stack here(in BL1) 
	 *	since the stack is already setup during BL0,
	 *	otherwise, the program may run abnormally
	 */
#ifdef BOOT_FROM_NAND
	 ldr sp, =8*1024
#endif

	bl	led_init

	@bl	uart_init

	bl	irq_init

	msr cpsr, #0x53

	/* Jump to myddr */
	bl	myuartint

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

not_used:
	b	not_used

irq:
	ldr sp, =4096
	
	sub	lr, lr, #4
	stmdb sp!, {r0-r12, lr}
	
	bl	do_irq
	
	ldmia sp!, {r0-r12, pc}^

fiq:
	b	fiq

