
/* NAND FLASH������ */
#define NFCONF (*((volatile unsigned long *)0x4E000000))
#define NFCONT (*((volatile unsigned long *)0x4E000004))
#define NFCMMD (*((volatile unsigned char *)0x4E000008))
#define NFADDR (*((volatile unsigned char *)0x4E00000C))
#define NFDATA (*((volatile unsigned char *)0x4E000010))
#define NFSTAT (*((volatile unsigned char *)0x4E000020))

/* GPIO */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

/* UART registers*/
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)

#define TXD0READY   (1<<2)


static void nand_read_ll(unsigned int addr, unsigned char *buf, unsigned int len);


static int isBootFromNorFlash(void)
{
	volatile int *p = (volatile int *)0;
	int val;

	val = *p;
	*p = 0x12345678;
	if (*p == 0x12345678)
	{
		/* д�ɹ�, ��nand���� */
		*p = val;
		return 0;
	}
	else
	{
		/* NOR�������ڴ�һ��д */
		return 1;
	}
}

void copy2ram(unsigned char *src, unsigned char *dest, unsigned int len)
{	
	int i = 0;
	
	/* �����NOR���� */
	if (isBootFromNorFlash())
	{
		while (i < len)
		{
			dest[i] = src[i];
			i++;
		}
	}
	else
	{
		nand_read_ll((unsigned int)src, dest, len);
	}
}

void clear_bss(void)
{
	extern int __bss_start, __bss_end__;
	int *p = &__bss_start;
	
	for (; p < &__bss_end__; p++)
		*p = 0;
}

void nand_init_ll(void)
{
#define TACLS   0
#define TWRPH0  1
#define TWRPH1  0
	/* ����ʱ�� */
	NFCONF = (TACLS<<12)|(TWRPH0<<8)|(TWRPH1<<4);
	/* ʹ��NAND Flash������, ��ʼ��ECC, ��ֹƬѡ */
	NFCONT = (1<<4)|(1<<1)|(1<<0);	
}

static void nand_select(void)
{
	NFCONT &= ~(1<<1);	
}

static void nand_deselect(void)
{
	NFCONT |= (1<<1);	
}

static void nand_cmd(unsigned char cmd)
{
	volatile int i;
	NFCMMD = cmd;
	for (i = 0; i < 10; i++);
}

static void nand_addr(unsigned int addr)
{
	unsigned int col  = addr % 2048;
	unsigned int page = addr / 2048;
	volatile int i;

	NFADDR = col & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR = (col >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	
	NFADDR  = page & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 8) & 0xff;
	for (i = 0; i < 10; i++);
	NFADDR  = (page >> 16) & 0xff;
	for (i = 0; i < 10; i++);	
}

static void nand_wait_ready(void)
{
	while (!(NFSTAT & 1));
}

static unsigned char nand_data(void)
{
	return NFDATA;
}

static void nand_read_ll(unsigned int addr, unsigned char *buf, unsigned int len)
{
	int col = addr % 2048;
	int i = 0;
		
	/* 1. ѡ�� */
	nand_select();

	while (i < len)
	{
		/* 2. ����������00h */
		nand_cmd(0x00);

		/* 3. ������ַ(��5������) */
		nand_addr(addr);

		/* 4. ����������30h */
		nand_cmd(0x30);

		/* 5. �ж�״̬ */
		nand_wait_ready();

		/* 6. ������ */
		for (; (col < 2048) && (i < len); col++)
		{
			buf[i] = nand_data();
			i++;
			addr++;
		}
		
		col = 0;
	}

	/* 7. ȡ��ѡ�� */		
	nand_deselect();
}

