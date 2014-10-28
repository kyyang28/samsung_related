
.globl _start

.globl bss_start
bss_start:
	.word __bss_start

.globl bss_end
bss_end:
	.word __bss_end

_start:
	/* 
	 *	iROM BL1 has already disabled watchdog, so that
	 *	acturally, we don't need to disable it again
	 */
	
	/* Clock asm init */
	bl	clock_asm_init

	/* DDR2 asm init */
	bl	ddr2_asm_init

	/* Copy codes from iSRAM to DDR2 */
relocate:
	adr r0, _start
	ldr r1, =_start
	cmp r0, r1
	beq	clear_bss

	ldr r2, bss_start

	/* The size of bin file stored in r2 */
	sub r2, r2, r1

	/* r3 = 0xD0020000 + sizeof(bin file) */
	add r3, r0, r2

	/* 
	 * If the linker script already skip the first 16 bytes, 
	 * then the cpu DONOT need to execute the next 2 instructions.
	 */
	/* Skip the first 16 bytes header info */
	add r0, r0, #0x10
	add r1, r1, #0x10

copy_loop:
	ldr r4, [r0], #4
	str r4, [r1], #4
	cmp r0, r3
	bne copy_loop

	/* Clear BSS Section */
clear_bss:
	ldr r0, bss_start
	ldr r1, bss_end
	cmp r0, r1
	beq jump2ddr2
	
	mov r2, #0x0

clear_bss_loop:
	str r2, [r0], #4
	cmp r0, r1
	bne clear_bss_loop

jump2ddr2:
	/* Switch mode from SVC to IRQ */
	mrs r0, cpsr
	bic r0, r0, #0x1F
	orr r0, r0, #0xD2
	msr cpsr, r0

	/* Setup the stack of IRQ on DDR2 */
	ldr sp, =0x3E000000

	/* Switch mode from IRQ to USR */
	mrs r0, cpsr
	bic r0, r0, #0x1F
	orr r0, r0, #0xD0
	and r0, r0, #~(1<<7)
	msr cpsr, r0

	/* Setup the stack of USR on DDR2 */
	ldr sp, =0x3F000000

	/* Jump to timer_int_main */
	ldr pc, =wdt_int_main

infinite_loop:
	b	infinite_loop

.globl do_irq
do_irq:
	sub lr, lr, #4

	stmdb sp!, {r0-r12, lr}

	bl	irq_handler

	ldmia sp!, {r0-r12, pc}^
	
