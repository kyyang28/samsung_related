
.text
.global _start
_start:
	ldr	sp, =4096
	
	bl	disable_watchdog
	bl	sdram_init
	bl	copy_from_SRAM_to_SDRAM
	ldr	pc, =_sdram
_sdram:
	ldr	sp, =0x34000000
	bl	led_init
	ldr 	lr, =infinite_loop
	ldr	pc, =mymain
infinite_loop:
	b	infinite_loop
	
