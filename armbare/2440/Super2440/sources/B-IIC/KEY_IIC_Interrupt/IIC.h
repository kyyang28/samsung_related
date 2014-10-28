
#ifndef __IIC_H__
#define __IIC_H__

typedef struct {
    unsigned char *data;      // buffer
    volatile int numOfData;   // buffer size
    volatile int mode;        // transmission mode
    volatile int status;      // transmission status: read/write
    volatile int pos;         // the position where the data to be transmitted
}/* __attribute__((__packed__)) */S3C2440_IIC;

S3C2440_IIC g_S3C2440_IIC;

void IIC_init(void);
void IIC_write(unsigned int slaveAddr, unsigned char *buf, int size);
void IIC_read(unsigned int slaveAddr, unsigned char *buf, int size);

#endif


