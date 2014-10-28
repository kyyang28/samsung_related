
.text
.global _start
_start:
	ldr r0, =0x53000000
	ldr r1, =0x4a21
	str r1, [r0]

	ldr r2, =0x53000008
	;ldr r3, =0x2710
	ldr r3, =0xEA60
	str r3, [r2]
	
	ldr sp, =1024*4
	
	bl 	mymain

halt_loop:
	b	halt_loop

