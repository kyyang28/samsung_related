

#define	WTCON					(0x53000000) 

.globl _start
_start:

	/* Disable watchdog */
	ldr	r0, =WTCON
	mov r1, #0x0
	str r1, [r0]
	
	/* Clock initialization */
	bl	clock_asm_init
	
	/* SDRAM initialization */
	bl	sdram_asm_init
	
	/* Relocation */
relocate:
	adr	r0, _start
	ldr r1, =_start		/* 0x30000000 in linker script */
	cmp r0, r1
	beq	clear_bss

	/* Setup the stack on SRAM */
	ldr	sp, =4096
	
	/* Nandflash initialization */
	bl	nand_init_ll
	
	ldr r2, =__bss_start
	sub r2, r2, r1
	
	bl	copy2ram
	
clear_bss:
	ldr	r0, =__bss_start
	ldr r1, =_end
	mov	r2, #0x0
	cmp r0, r1
	beq jump2sdram

cls_bss_loop:
	str r2, [r0], #4
	cmp r0, r1
	bne	cls_bss_loop

jump2sdram:
	ldr	sp, =0x34000000

	/* Jump to nand_leds_main */
	ldr pc, =nand_leds_main
	
infinite_loop:
	b	infinite_loop

