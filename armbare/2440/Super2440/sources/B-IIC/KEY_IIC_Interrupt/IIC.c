
//#include <stdio.h>
#include "s3c2440.h"
#include "IIC.h"


void IIC_init(void)
{
    /* 10 = IICSCL */
    GPECON |= (1<<29);
    GPECON &= ~(1<<28);

    /* 10 = IICSDA */
    GPECON |= (1<<31);
    GPECON &= ~(1<<30);
    
    /* Internal pull-up function is disabled */
    GPEUP |= 0xC000;

    /*  
     * Bit [4] = 1 ( IIC-bus data output bit )
     * Bit [7:6] ( DO NOT setup these two bit at this time, 
     *             otherwise the transmittion will start automatically
     *           )
     */
    IICSTAT = (1<<4);

    /*  
     * Bit [7] = 1 (Acknowledge bit - Enable)
     * Bit [6] = 0 (IICCLK = fPCLK /16)
     * Bit [5] = 1 (IIC-Bus Tx/Rx interrupt bit - Enable)
     * Bit [4] = 0 ( 1) No interrupt pending (when read).
     *               2) Clear pending condition &
     *                  Resume the operation (when write).
     *             )
     * Bit [3:0] = 0xF (Tx clock = IICCLK/(IICCON[3:0]+1))
     * PCLK = 50MHz; IICCLK = PCLK/16 = 3.125MHz; Tx clock = 3.125/(15+1) = 0.195MHz)
     */
    IICCON = (1<<7 | 0<<6 | 1<<5 | 0<<4 | 0xF);

}


void IIC_write(unsigned int slaveAddr, unsigned char *buf, int size)
{
    //printf("\r\nIIC_write function\r\n");    
    g_S3C2440_IIC.data = buf;
    g_S3C2440_IIC.numOfData = size;
    g_S3C2440_IIC.pos = 0;
    g_S3C2440_IIC.mode = WRITE;
	
    IICDS = slaveAddr;
    IICSTAT = 0xF0;

    /* Wait until the data is transmitted */
    while(g_S3C2440_IIC.numOfData != -1);
}


void IIC_read(unsigned int slaveAddr, unsigned char *buf, int size)
{
    g_S3C2440_IIC.data = buf;
    g_S3C2440_IIC.numOfData = size;
    g_S3C2440_IIC.mode = READ;
    g_S3C2440_IIC.pos = -1;  // In read mode, the first data is address.

    IICDS = slaveAddr;

    /* 
     * [7:6] = 10 Master/Receive mode
     * [5] = 1 START signal 
     * [4] = 1 Enable Rx/Tx (Data output enable)
     */
    IICSTAT = 0xB0;	  

    while(g_S3C2440_IIC.numOfData != 0);
}


