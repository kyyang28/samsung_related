
.globl _start
_start:

.globl bss_start
bss_start:
	.word __bss_start

.globl bss_end
bss_end:
	.word _bss_end

	/* 
	 *	iROM BL1 has already disabled watchdog, so that
	 *	acturally, we don't need to disable it again
	 */
	
	/* Clock init */
	bl	clock_asm_init
	
	/* DDR2 init */
	bl	ddr2_asm_init

	/* Copy codes from iSRAM to DDR2 */
relocate:
	adr	r0, _start		/* r0 = 0xD0020000, iSRAM starting address */
	ldr r1, =_start		/* r1 = linking address specified by the linker script */
	ldr	r2, bss_start	/* r1 = absolute address of the bss_start calculated by the linker script */
	cmp r0, r1
	beq	clear_bss

	/* r2 = sizeof(bin_file) */
	sub r2, r2, r1
	
	/* r3 = r0 + r2 = 0xD0020000 + sizeof(bin_file) */
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

	/* Clear BSS section */
clear_bss:
	ldr	r0, bss_start
	ldr r1, bss_end
	cmp r0, r1
	beq	jump2DDR2

	mov r2, #0x0

clear_bss_loop:
	str r2, [r0], #4
	cmp r0, r1
	bne clear_bss_loop

jump2DDR2:
	/* Setup the stack on DDR2 */
	ldr sp, =0x3F000000

	/* Jump to ddr2_leds_main */
	ldr	pc, =ddr2_leds_uart_main
	
_infinite_loop:
	b	_infinite_loop

