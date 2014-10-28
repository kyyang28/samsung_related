 
.equ	WTCON,		0x53000000

.text
.global _start
_start:
	ldr	sp, =4096                
	bl      disable_watchdog
	bl	clock_init       
        bl      sdram_init           
        bl      nand_init               

        ldr     r0,     =0x30000000     
        mov     r1,     #4096           
        mov     r2,     #2048           
        bl      nand_read               

        ldr     sp, =0x34000000         
        ldr     lr, =infinite_loop          
        ldr     pc, =mymain              

disable_watchdog:
	mov	r0, #0x0
	ldr	r1, =WTCON
	str	r0, [r1]
	mov	pc, lr

infinite_loop:
        b       infinite_loop

