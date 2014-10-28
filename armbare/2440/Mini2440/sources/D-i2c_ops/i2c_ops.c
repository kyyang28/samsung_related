
#include "s3c2440-regs.h"
#include "i2c_ops.h"

int i2c_init_ll(void)
{
	/* GPIO Setup */
	GPEUP |= 0xc000;
	GPECON |= 0xa0000000;

	/* IIC Registers */
	IICCON = ((1<<7) | (0<<6) | (1<<5) | (0<<4) | (0xf));
	IICADD = 0x10;
	IICSTAT = (1<<4);
	return 0;
}

void i2c_write_ll(u32 slaveAddr, u8 *buff, u32 len)
{
	i2c_buff	= buff;
	i2c_datacnt = len;
	i2c_mode	= WR_DATA;
	i2c_pos		= 0;

	IICDS	= slaveAddr;
	IICSTAT = ((0x3<<6) | (1<<5) | (1<<4));		/* MastTx, Start */

	while (i2c_datacnt != -1);	/* Wait until the data is finished sending*/
}

void i2c_read_ll(u32 slaveAddr, u8 *buff, u32 len)
{
	i2c_buff	= buff;
	i2c_datacnt	= len;
	i2c_mode	= RD_DATA;
	i2c_pos		= -1;

	IICDS	= slaveAddr;
	IICSTAT	= ((0x2<<6) | (1<<5) | (1<<4));

	while (i2c_datacnt != 0);
}

