		
.text
.global _start
_start:
		
		bl disable_watch_dog    
		
		ldr sp, =1024*4
		
		bl	mymain
		
halt_loop:
		b	halt_loop

disable_watch_dog:
	
	    mov  r1, #0x53000000
	    mov  r2, #0x0
	    str  r2, [r1]
	 
	    mov  pc, lr  
		
