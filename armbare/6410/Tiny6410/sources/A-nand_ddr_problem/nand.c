
#include "s3c6410_regs.h"

#if 0
/* 
 *	If level = 0, enable chip select
 *	If level = 1, disable chip select
 */
void nand_select(unsigned int level)
{
	NFCONT |= (level<<1);
}
#endif

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
	NFCMMD = cmd;
}

void nand_addr(unsigned char addr)
{
	NFADDR = addr;
}

unsigned char nand_getData(void)
{
	return NFDATA;
}

void wait4Ready(void)
{
	while (!(NFSTAT & 0x1));
}

void nand_reset(void)
{
	nand_select();
	nand_cmd(0xFF);
	wait4Ready();
	nand_deselect();
}

void nand_sendAddr(unsigned int addr)
{
	/* 5 cycles to send the addresses */
#if 0
	nand_addr(addr & 0xFF);			/* Column address */
	nand_addr((addr>>8) & 0xF);		/* Column address */
	nand_addr((addr>>12) & 0xFF);	/* Page address */
	nand_addr((addr>>20) & 0xFF);	/* Page address */
	nand_addr((addr>>28) & 0xFF);	/* Page address */
#else
	unsigned int page = addr / 4096;

	/* Where to start within 1 page? */
	nand_addr(addr & 0xFF);			/* Column address */
	nand_addr((addr>>8) & 0xFF);	/* Column address */

	/* Which page? */
	nand_addr(page & 0xFF);			/* Page address */
	nand_addr((page>>8) & 0xFF);	/* Page address */
	nand_addr((page>>16) & 0xFF);	/* Page address */
#endif
}

void nand_init_ll(void)
{
	/* NFCON CS0 setup to low level */
	MEM_SYS_CFG &= ~(1<<1);
	
	/* Setup the NFCONF register */
	NFCONF &= ~((0x7<<4) | (0x7<<8) | (0x7<<12) | (1<<30));
	NFCONF |= ((TWRPH1<<4) | (TWRPH0<<8) | (TACLS<<12));
	
	/* Setup the NFCONT register - Enable nandflash controller */
	NFCONT |= 0x1;
	
	nand_reset();
}

int nand_read_ll(unsigned int nandBegin, unsigned char *ddrBegin, unsigned int size)
{
	extern void led_on(int);
	//int i, cnt = 0;
	int i, j;
	//unsigned int nandAddr  = nandBegin;
	
	nand_select();

#if 0
	while (cnt < size) {
		nand_cmd(0x00);
		nand_sendAddr(nandAddr);
		nand_cmd(0x30);
		wait4Ready();

		for (i = 0; i < 4096 && cnt < size; i++) {
			ddrAddr[cnt++] = nand_getData();
			led_on(3);
		}
		
		nandAddr += 4096;
	}
#else
	for (i = nandBegin; i < (nandBegin + size);) {
		nand_cmd(0x0);
		nand_sendAddr(i);
		nand_cmd(0x30);
		wait4Ready();
		
		for (j = 0; j < 4096; j++,i++) {
			*ddrBegin = nand_getData();
			ddrBegin++;
		}
	}
#endif

	nand_deselect();
	return 0;
}

int copyCode2DDR(unsigned int nandBegin, unsigned char *ddrBegin, unsigned int size)
{
	int ret;

	/* Initialize nandflash */
	nand_init_ll();

	/* Read codes from nandflash to DDR */
	ret = nand_read_ll(nandBegin, ddrBegin, size);

	return ret;
}


