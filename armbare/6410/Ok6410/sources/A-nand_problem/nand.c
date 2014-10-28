

#define MEM_SYS_CFG     (*((volatile unsigned long *)0x7E00F120))
#define NFCONF          (*((volatile unsigned long *)0x70200000))
#define NFCONT          (*((volatile unsigned long *)0x70200004))
#define NFCMMD          (*((volatile unsigned long *)0x70200008))
#define NFADDR          (*((volatile unsigned long *)0x7020000C))
#define NFDATA          (*((volatile unsigned char *)0x70200010))
#define NFSTAT          (*((volatile unsigned long *)0x70200028))


void nand_select(void)
{
	NFCONT &= ~(1<<1);
}

void nand_deselect(void)
{
	NFCONT |= (1<<1);
}


void nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);
}

void nand_addr(unsigned char addr)
{
	volatile int i;
	NFADDR = addr;
	for (i = 0; i < 10; i++);
}

unsigned char nand_get_data(void)
{
	return NFDATA;
}

void wait_ready(void)
{
	while ((NFSTAT & 0x1) == 0);
}

void nand_reset(void)
{
	/* 选中 */
	nand_select();
	
	/* 发出0xff命令 */
	nand_cmd(0xff);

	/* 等待就绪 */
	wait_ready();
	
	/* 取消选中 */
	nand_deselect();
}


void nand_init(void)
{
	/* 让xm0csn2用作nand flash cs0 片选引脚 */
	MEM_SYS_CFG &= ~(1<<1);

	/* 设置时间参数 */
#define TACLS     0
#define TWRPH0    1
#define TWRPH1    0
	NFCONF &= ~((1<<30) | (7<<12) | (7<<8) | (7<<4));
	NFCONF |= ((TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4));

	/* 使能nand flash controller */
	NFCONT |= 1;

	nand_reset();
}


void nand_send_addr(unsigned int addr)
{
	volatile int i;
#if 1	
	unsigned int page = addr / 2048;
	unsigned int column = addr % 2048;

	/* 这两个地址表示从页内哪里开始 */
	nand_addr(column & 0xff);
	for (i = 0; i < 10; i++);
	nand_addr((column >> 8) & 0xff);
	for (i = 0; i < 10; i++);

	/* 下面三个地址表示哪一页 */
	nand_addr(page & 0xff);
	for (i = 0; i < 10; i++);
	nand_addr((page >> 8) & 0xff);
	for (i = 0; i < 10; i++);
	nand_addr((page >> 16) & 0xff);
	for (i = 0; i < 10; i++);
#else
#if 0
	nand_addr(addr & 0xff);         /* a0~a7 */
	nand_addr((addr >> 8) & 0x7);   /* 程序的角度: a8~a10 */

	nand_addr((addr >> 11) & 0xff); /* 程序的角度: a11~a18 */
	nand_addr((addr >> 19) & 0xff); /* 程序的角度: a19~a26 */
	nand_addr((addr >> 27) & 0xff); /* 程序的角度: a27~    */
#else
	nand_addr(addr & 0xff);         /* a0~a7 */
	nand_addr((addr >> 8) & 0x1f);   /* 程序的角度: a8~a12 */

	nand_addr((addr >> 13) & 0xff); /* 程序的角度: a13~a20 */
	nand_addr((addr >> 21) & 0xff); /* 程序的角度: a21~a28 */
	nand_addr((addr >> 29) & 0xff); /* 程序的角度: a29~    */
#endif
#endif
}


int nand_read(unsigned int nand_start, unsigned int ddr_start, unsigned int len)
{
	unsigned int addr = nand_start;
	int i, count = 0;
	unsigned char *dest = (unsigned char *)ddr_start;
	
	/* 选中芯片 */
	nand_select();

	while (count < len)
	{
		/* 发出命令0x00 */
		nand_cmd(0x00);

		/* 发出地址 */
		nand_send_addr(addr);

		/* 发出命令0x30 */
		nand_cmd(0x30);

		/* 等待就绪 */
		wait_ready();

		/* 读数据 */
		for (i = 0; i < 2048 && count < len; i++)
		//for (i = 0; i < 4096 && count < len; i++)
		{
			dest[count++] = nand_get_data();
		}

		//addr += 4096;			
		addr += 2048;			
	}

	/* 取消片选 */
	nand_deselect();
	return 0;
}

int copy2ddr(unsigned int nand_start, unsigned int ddr_start, unsigned int len)
{
	int ret;
	
	/* 初始化nand flash controller */
	//nand_init();
	
	/* 读nand flash */
	ret = nand_read(nand_start, ddr_start, len);
	
	return ret;
}

