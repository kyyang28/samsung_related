

#include "IIC.h"


void eeprom_write(unsigned char address, unsigned char data)
{
    unsigned char buf[2];
    buf[0] = address;
    buf[1] = data;
    IIC_write(0xA0, buf, 2);
}

unsigned char eeprom_read(unsigned char address)
{
    unsigned char res;
    IIC_write(0xA0, &address, 1);
    IIC_read(0xA0, (unsigned char *)&res, 1);
    return res;
}


