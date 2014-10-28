
#include "s3c2440-regs.h"
#include "i2c_ops.h"

u8 eeprom_read_ll(u8 addr)
{
	u8 res;
	i2c_write_ll(0xA0, &addr, 1);
	i2c_read_ll(0xA0, (u8 *)&res, 1);
	return res;
}

void eeprom_write_ll(u8 addr, u8 val)
{
	u8 res[2];
	res[0] = addr;
	res[1] = val;
	i2c_write_ll(0xA0, res, 2);
}

