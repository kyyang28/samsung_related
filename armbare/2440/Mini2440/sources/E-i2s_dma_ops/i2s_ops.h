#ifndef __I2S_OPS_H
#define __I2S_OPS_H

int i2s_init_ll(void);
void i2s_write_ll(short * pdata, int data_size);
void i2s_read_ll(short * pdata, int data_size);

#endif

