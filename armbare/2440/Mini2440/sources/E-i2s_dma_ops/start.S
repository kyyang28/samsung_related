
.globl _start
_start:
	
.globl bss_start
bss_start:
	.word __bss_start

.globl bss_end
bss_end:
	.word _end

#define WTCON	0x53000000
	
	/* Disable watchdog */
	ldr	r0, =WTCON
	mov	r1, #0x0
	str r1, [r0]
	
	/* Clock initialization */
	bl	clock_asm_init

	/* Sdram initialization */
	bl	sdram_asm_init

	/* Setup the stack on SRAM */
	ldr	sp, =4096

	/* Relocation */
relocate:
	adr	r0, _start
	ldr	r1, =_start
	cmp	r0, r1
	beq	clear_bss
	
	/* Nandflash initialization */
	bl	nand_init_ll
	
	ldr	r2, bss_start
	sub	r2, r2, r1
	
	/* Copy codes from nandflash to sdram */
	bl	copy2sdram
	
	/* Clear bss */
clear_bss:
	ldr	r0, bss_start
	ldr	r1, bss_end
	cmp r0, r1
	beq	jump2sdram

	mov	r2, #0x0

cls_bss_loop:
	str r2, [r0], #4
	cmp r0, r1
	bne cls_bss_loop

jump2sdram:

	/* Save the return address of the i2s_main */
	ldr	lr, =infinite_loop
	
	/* Jump to i2s_main */
	ldr	pc, =i2s_main
	
infinite_loop:
	b	infinite_loop

