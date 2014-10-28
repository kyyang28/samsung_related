#ifndef __EEPROM_OPS_H
#define __EEPROM_OPS_H

#include "s3c2440-regs.h"

u8 eeprom_read_ll(u8 addr);
void eeprom_write_ll(u8 addr, u8 val);
void i2c_eeprom_test(void);

#endif

