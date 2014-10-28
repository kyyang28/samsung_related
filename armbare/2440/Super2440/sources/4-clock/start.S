
.text
.global _start
_start:
    
    ldr	sp, =4096		@ SRAM size	
	
	bl	disable_watchdog
	bl	clock_init
	bl	led_init
	bl	key_init

	bl	mymain

infinite_loop:
	b	infinite_loop

