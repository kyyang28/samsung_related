

#include "s3c2440-regs.h"

void nand_init_ll(void)
{
	NFCONF = NAND_NFCONF_TACL<<12 | NAND_NFCONF_TWRPH0<<8 | NAND_NFCONF_TWRPH1<<4;
	NFCONT = NAND_NFCONT_CHIP_DISABLE | NAND_NFCONT_CONTROLLER_ENABLE;
}

static void nand_chip_select_ll(void)
{
	NFCONT &= ~(1<<1);
}

static void nand_chip_deselect_ll(void)
{
	NFCONT |= (1<<1);
}

static void nand_send_cmd_ll(u8 cmd)
{
	NFCMMD = cmd;
}

static void nand_send_addr_ll(u32 addr)
{
	unsigned int column = addr % NAND_PAGE_SIZE;
	unsigned int row = addr / NAND_PAGE_SIZE;
	
	NFADDR = column & 0xFF;
	NFADDR = (column >> 8) & 0xFF;
	NFADDR = row & 0xFF;
	NFADDR = (row >> 8) & 0xFF;
	NFADDR = (row >> 16) & 0xFF;
}

static int nand_check_status_ll(void)
{
	return NFSTAT & NAND_NFSTAT_STATUS;
}

static u8 nand_send_data_ll(void)
{
	return NFDATA;
}

static void nand_read_ll(u32 nand_addr, u8 *sdram_addr, u32 len)
{
	int index = 0;

	u32 column = nand_addr % NAND_PAGE_SIZE;

	/* Nandflash chip select */
	nand_chip_select_ll();

	while (index < len) {
		/* Nandflash send read command 0x00 */
		nand_send_cmd_ll(0x00);

		/* Nandflash send address */
		nand_send_addr_ll(nand_addr);

		/* Nandflash send read command 0x30 */
		nand_send_cmd_ll(0x30);

		/* Nandflash check status */
		while (!nand_check_status_ll());

		while ((column < NAND_PAGE_SIZE) && (index < len)) {
			/* Nandflash send data */
			sdram_addr[index] = nand_send_data_ll();
			column++;
			nand_addr++;
			index++;
		}

		column = 0;
	}

	/* Nandflash chip deselect */
	nand_chip_deselect_ll();

}

void copy2sdram(u32 nand_addr, u8 *sdram_addr, u32 len)
{
	/* Nandflash read operation */
	nand_read_ll(nand_addr, sdram_addr, len);
}

