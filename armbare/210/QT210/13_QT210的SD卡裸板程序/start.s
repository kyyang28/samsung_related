
.global _start
	
_start:
	ldr sp, =0xD0030000	@��ʼ����ջ
	bl ddr_init                   /* ��ʼ���ڴ� */

	b main

