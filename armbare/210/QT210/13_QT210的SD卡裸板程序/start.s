
.global _start
	
_start:
	ldr sp, =0xD0030000	@初始化堆栈
	bl ddr_init                   /* 初始化内存 */

	b main

