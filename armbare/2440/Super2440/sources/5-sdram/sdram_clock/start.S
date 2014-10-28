
.text
.global _start
_start:
	ldr	sp, =4096
	
	bl	disable_watchdog
	bl	clock_init
	bl	led_init
	bl	sdram_init
	bl	copy_from_SRAM_to_SDRAM
	
    	ldr 	sp, =0x34000000
    	ldr	lr, =infinite_loop
	ldr	pc, =mymain

infinite_loop:
	b	infinite_loop

