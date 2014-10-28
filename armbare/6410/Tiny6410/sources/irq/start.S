
.globl _start
_start:

	/* 0 地址 */
	b reset                 /* 复位时,cpu跳到0地址 */
	ldr pc, =undefined_instruction  /* cpu遇到不能识别的指令时 */
	ldr pc, _vector_swi             /* 当执行swi指令时, 进入swi模 式 */
	b halt     @ldr	pc, _prefetch_abort /* 预取中止异常 */
	b halt     @ldr	pc, _data_abort     /* 数据访问异常 */
	b halt     @ldr	pc, _not_used       /* 没用到 */
	ldr	pc, _irq            /* 0x18 中断异常 */
	b halt     @ldr	pc, _fiq            /* 快中断异常 */

_irq :
	.word vector_irq

_vector_swi:
	.word vector_swi
		
vector_swi:
	/* 1. 保存现场 */
	ldr sp, =0x56000000
	stmdb sp!, {r0-r12, lr}  /* lr就是swi的下一条指令地址 */

	/* 2. 处理异常 */
	mrs r0, cpsr
	ldr r1, =swi_str
	bl print_cpsr

	/* 3. 恢复现场 */
	ldmia sp!, {r0-r12, pc}^ /* ^表示把spsr恢复到cpsr */
	
swi_str:
	.word 0x00697773  /* swi */
	
undefined_instruction:
	/* 1. 保存现场 */
	ldr sp, =0x55000000
	stmdb sp!, {r0-r12, lr}

	/* 2. 处理异常 */
	mrs r0, cpsr
	ldr r1, =und_str
	bl print_cpsr

	/* 3. 恢复现场 */
	ldmia sp!, {r0-r12, pc}^  /* ^表示把spsr恢复到cpsr */

und_str:
	.word 0x00646e75  /* und */

usr_str:
	.word 0x00727375  /* usr */


vector_irq:
	/* 1. 保存现场 */
	ldr sp, =0x54000000
	sub lr, lr, #4
	stmdb sp!, {r0-r12, lr}  /* lr就是swi的下一条指令地址 */

	/* 2. 处理异常 */
	bl do_irq
	
	/* 3. 恢复现场 */
	ldmia sp!, {r0-r12, pc}^  /* ^表示把spsr恢复到cpsr */

reset:
/* 硬件相关的设置 */
    /* Peri port setup */
    ldr r0, =0x70000000
    orr r0, r0, #0x13
    mcr p15,0,r0,c15,c2,4       @ 256M(0x70000000-0x7fffffff)
    
/* 关看门狗 */
/* 往WTCON(0x7E004000)写0 */
	
	ldr r0, =0x7E004000
	mov r1, #0
	str r1, [r0]
	
	/* 设置栈 */
	ldr sp, =8*1024

	/* 设置时钟 */
	bl clock_init
	bl ddr_init
	bl init_uart

/* 把程序的代码段、数据段复制到它的链接地址去 */	
	adr r0, _start   /* 获得_start指令当前所在的地址 : 0*/
	ldr r1, =_start  /* _start的链接地址 0x51000000 */
	
	ldr r2, =bss_start      /* bss段的起始链接地址 */
	
	sub r2, r2, r1
	
	cmp r0,r1
	beq clean_bss
	
	bl copy2ddr
	cmp r0, #0
	bne halt
		
	/* 清BSS */
/* 把BSS段对应的内存清零 */
clean_bss:
	ldr r0, =bss_start
	ldr r1, =bss_end
	mov r3, #0
	cmp r0, r1
	ldreq pc, =on_ddr
clean_loop:
	str r3, [r0], #4
	cmp r0, r1	
	bne clean_loop		
	ldr pc, =on_ddr

on_ddr:	

	bl irq_init

	mrs r0, cpsr
	bic	r0,r0,#0x9f  /* 清cpsr的I位,M4~M0 */
	orr	r0,r0,#0x10
	msr	cpsr,r0      /* 进入user mode */

	ldr sp, =0x57000000

	ldr r1, =usr_str
	bl print_cpsr

#if 0
	swi 0      /* 
	          * cpu进入svc模式
	          * 把之前的cpsr保存到spsr_svc 
	          * 切换到r13_svc, r14_svc
	          * 把swi的下一条指令存到r14(lr)_svc
	          * 跳到地址8
	          */
	bl hello
#endif

undef:
	.word 0xff000000 /*
			          * cpu进入Undefined模式
			          * 把之前的cpsr保存到spsr_und 
			          * 切换到r13_und, r14_und
			          * 把下一条指令存到r14(lr)_und
			          * 跳到地址4
			          */
	
	bl main
	

halt:
	b halt	


