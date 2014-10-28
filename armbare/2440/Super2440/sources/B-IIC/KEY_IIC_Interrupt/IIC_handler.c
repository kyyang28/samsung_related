
#include <stdio.h>
#include "s3c2440.h"
#include "IIC.h"


static inline void DELAY (unsigned long ticks)
{
    __asm__ volatile (
        "1:\n"
        "subs %0, %1, #1\n"
	"bne 1b":"=r" (ticks):"0" (ticks));
}


void do_IIC_ISR(void)
{
    /* Clear IIC interrupt */
    SRCPND = (1<<27);
    INTPND = (1<<27);

   	if(IICSTAT & 0x8)
            printf("\r\nBus arbitration failed during serial I/O\r\n");
    	else if(IICSTAT & 0x4)
            printf("\r\nReceived slave address matches the address value in the IICADD\r\n");
    	else if(IICSTAT & 0x2)
            printf("\r\nReceived slave address is 00000000b\r\n");
    	else if(IICSTAT & 0x1)
            printf("\r\nLast-received bit is 1 (ACK was not received)\r\n");
    
        switch(g_S3C2440_IIC.mode) {
    
            case WRITE:
            {
                /* Case 1: The transmission is finished */
                if( 0 == (g_S3C2440_IIC.numOfData--) ) {
                    /* 
    	             * [7:6] = 11 Master/Transmit mode
    	             * [5] = 0 STOP signal 
    	             * [4] = 1 Enable Rx/Tx (Data output enable)
    	             */
    				IICSTAT = 0xD0;
    
    				/* 
    	             * [7] = 1 Enable acknowledge bit
                     * [6] = 0 IICCLK = PCLK/16 
    	             * [5] = 1 Tx/Rx interrupt enable 
    	             * [4] = 0 Clear pending bit & RESUME THE OPERATION!!(When write)
    	             * [3:0] = 0xF
    	             */
    	            IICCON = 0xAF;
    				DELAY(10000);
    
                    break;
                }
    
    			/* Case 2: The transmission is not finished */
    			IICDS = g_S3C2440_IIC.data[g_S3C2440_IIC.pos++];
    
                /* 
                 * After the data is put into the IICDS register, we need to
                 * wait until it appeares on the SDA line.
                 */
    			DELAY(10);
    
                /* Resume IIC transmission */
                IICCON = 0xAF;
    			break;
            }
    		case READ:
            {
                /* The first data received is address, so DO NOT send ACK */
                if( -1 == g_S3C2440_IIC.pos) {
    
    				g_S3C2440_IIC.pos = 0;
    
    				if( 1 == g_S3C2440_IIC.numOfData ) 
                        /* 
    	                 * [7] = 0 Disable acknowledge bit
                         * [6] = 0 IICCLK = PCLK/16 
    	                 * [5] = 1 Tx/Rx interrupt enable 
           	             * [4] = 0 Clear pending bit & RESUME THE OPERATION!!(When write)
    	                 * [3:0] = 0xF
    	                 */
    	                IICCON = 0x2F;  // Resume IIC transmission, W/O ACK
    				else
    					IICCON = 0xAF;  // Resume IIC transmission, W/ ACK
                    break;
                }
    
                g_S3C2440_IIC.data[g_S3C2440_IIC.pos++] = IICDS;
    			g_S3C2440_IIC.numOfData--;
    
    			if( 0 == g_S3C2440_IIC.numOfData ) {
                    /* 
    	             * [7:6] = 10 Master/Receive mode
    	             * [5] = 0 STOP signal 
    	             * [4] = 1 Enable Rx/Tx (Data output enable)
    	             */
    	            IICSTAT = 0x90;
    
                    /* 
    	             * [7] = 1 Enable acknowledge bit
                     * [6] = 0 IICCLK = PCLK/16 
    	             * [5] = 1 Tx/Rx interrupt enable 
    	             * [4] = 0 Clear pending bit & RESUME THE OPERATION!!(When write)
    	             * [3:0] = 0xF
    	             */
    	            IICCON = 0xAF;
                    DELAY(10000);
    
    				
                    break;
    			}else {
                    if( 1 == g_S3C2440_IIC.numOfData)
                        IICCON = 0x2F;
    				else
                        IICCON = 0xAF;
                    break;
    			}
            }
    
    		default:
                break;
        }
}


