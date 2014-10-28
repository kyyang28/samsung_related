

.globl bss_start
bss_start:
	.word __bss_start

.globl bss_end
bss_end:
	.word _end

.globl _start
_start:

#define WTCON		(0x53000000) 
	/* Disable watchdog */
	ldr	r0, =WTCON
	mov	r1, #0x0
	str r1, [r0]
	
	/* Clock initialization */
	bl	clock_asm_init
	
	/* SDRAM initialization */
	bl	sdram_asm_init
	
	/* Relocation */
relocate:
	adr	r0, _start
	ldr r1, =_start
	cmp	r0, r1
	beq	clear_bss
	
	/* Setup the stack on SRAM */
	ldr	sp, =4096
	
	/* Nandflash initialization */
	bl	nand_init_ll
	
	ldr	r2, bss_start
	sub r2, r2, r1

	/* Copy programs from nandflash to sdram */
	bl	copy2sdram
	
clear_bss:
	ldr	r0, bss_start
	ldr	r1, bss_end
	mov	r2, #0x0
	cmp r0, r1
	beq	jump2sdram
	
clear_bss_loop:
	str r2, [r0], #4
	cmp r0, r1
	bne clear_bss_loop
	
jump2sdram:

	/* Save the return address of rtc_main */
	ldr	lr,	=infinite_loop
	
	/* Jump to rtc_main */
	ldr	pc, =rtc_main
	
infinite_loop:
	b	infinite_loop

