
.text
.global _start
_start:
	b	reset
	
_undefined_instruction:
	b	_undefined_instruction
	
_software_interrupt:
	b	_software_interrupt
	
_prefetch_abort:
	b	_prefetch_abort
	
_data_abort:
	b	_data_abort
	
	b	_irq
	
_fiq:
	b	_fiq
	
reset:
	ldr	sp, =4096
	bl	disable_watchdog
	
	msr	cpsr_c,	#0xD2	// IRQ
	ldr	sp, =3072
	
	msr	cpsr_c, #0xD3	// SVC
	ldr	sp, =4096
	
	bl	led_init
	mov	r0, #0
	bl	uart_init
	bl	irq_init
	
	ldr	lr,	=infinite_loop
	ldr	pc,	=mymain
	
infinite_loop:
	b	infinite_loop
	
_irq:
	sub	lr,	lr,	#4
	stmdb	sp!, {r0-r12, lr}
	
	ldr	lr,	=_interrupt_return
	ldr	pc, =IRQ_Handle
	
_interrupt_return:
	ldmia	sp!, {r0-r12, pc}^
