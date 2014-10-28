#ifndef __I2C_OPS_H
#define __I2C_OPS_H

#include "s3c2440-regs.h"

unsigned char *i2c_buff;
volatile int i2c_datacnt;
volatile int i2c_status;
volatile int i2c_mode;
volatile int i2c_pos;

int i2c_init_ll(void);
void i2c_write_ll(u32 slaveAddr, u8 *buff, u32 len);
void i2c_read_ll(u32 slaveAddr, u8 *buff, u32 len);

#endif

