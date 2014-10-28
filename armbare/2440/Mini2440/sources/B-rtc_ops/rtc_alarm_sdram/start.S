

.globl _start
_start:
	b	start_code
	ldr	pc, _undefined_instruction
	ldr	pc, _software_interrupt
	ldr	pc, _prefetch_abort
	ldr	pc, _data_abort
	ldr	pc, _reserved
	ldr	pc, _irq
	ldr	pc, _fiq

_undefined_instruction:		.word undefined_instruction
_software_interrupt:		.word software_interrupt
_prefetch_abort:			.word prefetch_abort
_data_abort:				.word data_abort
_reserved:					.word reserved
_irq:						.word irq
_fiq:						.word fiq


.globl bss_start
bss_start:
	.word __bss_start

.globl bss_end
bss_end:
	.word _end

	
start_code:

#define WTCON		(0x53000000) 
	/* Disable watchdog */
	ldr	r0, =WTCON
	mov	r1, #0x0
	str r1, [r0]
	
	/* Clock initialization */
	bl	clock_asm_init
	
	/* SDRAM initialization */
	bl	sdram_asm_init
	
	/* Setup the stack on SRAM */
	ldr	sp, =4096
	
	/* Relocation */
relocate:
	adr	r0, _start
	ldr r1, =_start
	cmp	r0,	r1
	beq	clear_bss
	
	/* Nandflash initialization */
	bl	nand_init_ll
	
	ldr	r2, bss_start
	sub r2, r2, r1

	/* Copy programs from nandflash to sdram */
	bl	copy2sdram

clear_bss:
	ldr	r0, bss_start
	ldr	r1, bss_end
	cmp	r0, r1
	beq	jump2sdram

	mov	r2, #0x0

clear_bss_loop:
	str r2, [r0], #4
	cmp r0, r1
	bne clear_bss_loop

jump2sdram:

	/* Switch to IRQ mode and setup the stack */
	mrs	r0, cpsr
	bic	r0, r0, #0x1f
	orr r0, r0, #0xd2
	msr	cpsr, r0
	ldr	sp, =0x31000000

	/* Switch to SVC mode and setup the stack */
	mrs r0, cpsr
	bic r0, r0, #0x1f
	orr r0, r0, #0xd3
	msr	cpsr, r0
	ldr	sp, =0x34000000
	
	/* Save the return address of rtc_main */
	ldr	lr,	=infinite_loop

	/* Jump to rtc_main */
	ldr	pc, =rtc_main
	
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
	sub	lr, lr, #4
	stmdb sp!, { r0-r12, lr }

	ldr	lr, =isr_ret
	ldr	pc, =rtc_int_handler
isr_ret:
	ldmia sp!, { r0-r12, pc }^

fiq:
	b	fiq

