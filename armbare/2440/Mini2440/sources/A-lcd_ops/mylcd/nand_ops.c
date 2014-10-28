

#define NFCONF							(*(volatile unsigned long *)0x4E000000)
#define NFCONT							(*(volatile unsigned long *)0x4E000004)
#define NFCMMD							(*(volatile unsigned char *)0x4E000008)
#define NFADDR							(*(volatile unsigned char *)0x4E00000C)
#define	NFDATA							(*(volatile unsigned char *)0x4E000010)
#define NFSTAT							(*(volatile unsigned char *)0x4E000020)

#define TACLS							0
#define TWRPH0							1
#define	TWRPH1							0

#define NAND_PAGE_SIZE					2048


void nand_init_ll(void)
{
	NFCONF = TACLS<<12 | TWRPH0<<8 | TWRPH1<<4;
	NFCONT = 0x1<<1 | 0x1;
}

static void nand_chip_select_ll(void)
{
	NFCONT &= ~(1<<1);
}

static void nand_chip_deselect_ll(void)
{
	NFCONT |= (1<<1);
}

static void nand_send_cmd_ll(unsigned char cmd)
{
	NFCMMD = cmd;
}

static void nand_send_addr_ll(unsigned int addr)
{
	unsigned int column = addr % NAND_PAGE_SIZE;
	unsigned int row = addr / NAND_PAGE_SIZE;

	NFADDR = column & 0xFF;
	NFADDR = (column >> 8) & 0xFF;
	NFADDR = row & 0xFF;
	NFADDR = (row >> 8) & 0xFF;
	NFADDR = (row >> 16) & 0xFF;
}

static unsigned char nand_send_data_ll(void)
{
	return NFDATA;
}

static int nand_check_status_ll(void)
{
	return NFSTAT & (1<<0);
}

void nand_read_ll(unsigned int nand_addr, unsigned char *sdram_addr, unsigned int len)
{
	int index = 0;

	unsigned int column = nand_addr % NAND_PAGE_SIZE;

	/* Chip select */
	nand_chip_select_ll();

	while (index < len) {
	
		/* Send read command 0x00 */
		nand_send_cmd_ll(0x00);
	
		/* Send read address */
		nand_send_addr_ll(nand_addr);

		/* Send read command 0x30 */
		nand_send_cmd_ll(0x30);

		while (!nand_check_status_ll());

		while ((column < NAND_PAGE_SIZE) && (index < len)) {
			/* Send read data */
			sdram_addr[index] = nand_send_data_ll();
			column++;
			index++;
			nand_addr++;
		}

		column = 0;
	}
	/* Chip deselect */
	nand_chip_deselect_ll();
	
}

void copy2ram(unsigned int nand_addr, unsigned char *sdram_addr, unsigned int len)
{
	nand_read_ll(nand_addr, sdram_addr, len);
}

