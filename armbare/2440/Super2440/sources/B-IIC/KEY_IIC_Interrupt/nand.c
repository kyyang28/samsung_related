
#include "s3c2440.h"


static void s3c2440_nand_reset(void);
static void s3c2440_wait_idle(void);
static void s3c2440_nand_select_chip(void);
static void s3c2440_nand_deselect_chip(void);
static void s3c2440_write_cmd(int cmd);
//static void s3c2440_write_addr(unsigned int addr);
static void s3c2440_write_addr_lp(unsigned int addr);
static unsigned char s3c2440_read_data(void);

/* @@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@ */

void nand_reset(void)
{
    s3c2440_nand_reset();
}


void wait_idle(void)
{
    s3c2440_wait_idle();
}


void nand_select_chip(void)
{
    int i;
    s3c2440_nand_select_chip();
    for(i=0; i<10; i++);
}


void nand_deselect_chip(void)
{
    s3c2440_nand_deselect_chip();
}


void write_cmd(int cmd)
{
    s3c2440_write_cmd(cmd);
}


void write_addr(unsigned int addr)
{
#ifdef LARGER_NAND_PAGE
	s3c2440_write_addr_lp(addr);
#else
	s3c2440_write_addr(addr);
#endif
}


unsigned char read_data(void)
{
    return s3c2440_read_data();
}


static void s3c2440_nand_reset(void)
{
    s3c2440_nand_select_chip();
    s3c2440_write_cmd(0xff);  
    s3c2440_wait_idle();
    s3c2440_nand_deselect_chip();
}


static void s3c2440_wait_idle(void)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;
    
	int i;
    volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFSTAT;
    while(!(*p & BUSY))
        for(i=0; i<10; i++);
}


static void s3c2440_nand_select_chip(void)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;

    int i;
    s3c2440nand->NFCONT &= ~(1<<1);
    for(i=0; i<10; i++);    
}


static void s3c2440_nand_deselect_chip(void)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;

    s3c2440nand->NFCONT |= (1<<1);
}


static void s3c2440_write_cmd(int cmd)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;

    volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFCMD;
    *p = cmd;
}

#if 0
static void s3c2440_write_addr(unsigned int addr)
{
    int i;
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;

	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;
    
    *p = addr & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 9) & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 17) & 0xff;
    for(i=0; i<10; i++);
    *p = (addr >> 25) & 0xff;
    for(i=0; i<10; i++);
}
#endif

static void s3c2440_write_addr_lp(unsigned int addr)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;

	int i;
	volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFADDR;
	int col, page;

	col = addr & NAND_BLOCK_MASK_LP;
	page = addr / NAND_SECTOR_SIZE_LP;
	
	*p = col & 0xff;			/* Column Address A0~A7 */
	for(i=0; i<10; i++);		
	*p = (col >> 8) & 0x0f; 	/* Column Address A8~A11 */
	for(i=0; i<10; i++);
	*p = page & 0xff;			/* Row Address A12~A19 */
	for(i=0; i<10; i++);
	*p = (page >> 8) & 0xff;	/* Row Address A20~A27 */
	for(i=0; i<10; i++);
	*p = (page >> 16) & 0x03;	/* Row Address A28~A29 */
	for(i=0; i<10; i++);
}


static unsigned char s3c2440_read_data(void)
{
    S3C2440_NAND_REG * const s3c2440nand = (S3C2440_NAND_REG *)0x4E000000;

    volatile unsigned char *p = (volatile unsigned char *)&s3c2440nand->NFDATA;
    return *p;
}


int nand_read(unsigned char *buf, unsigned long start_addr, int size)
{
    int i, j;

	nand_select_chip();

	for(i = start_addr; i < (start_addr+size);) {
            write_cmd(0x0);
	    write_addr(i);

#ifdef LARGER_NAND_PAGE
		write_cmd(0x30);
#endif
		wait_idle();

#ifdef LARGER_NAND_PAGE
		for(j = 0; j < NAND_SECTOR_SIZE_LP; j++, i++) {
#else
		for(j = 0; j < NAND_SECTOR_SIZE; j++, i++) {
#endif
			*buf = read_data();
			buf++;
		}
	}

	nand_deselect_chip();

	return 0;
}



