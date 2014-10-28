
#include <stdio.h>
#include "s3c2440-regs.h"
#include "lib_ops.h"
#include "i2c_ops.h"


void i2c_int_init_ll(void)
{
	/* Setup the interrupt controller */
	INTMOD &= ~INT_IIC;
	INTMSK &= ~INT_IIC;

	/* IMPORTANT: Enable the I-bit of CPSR */
	__asm__ (
		"mrs r0, cpsr\n"
		"and r0, r0, #~(1<<7)\n"
		"msr cpsr, r0\n"
	);
}

void i2c_isr(void)
{
	int i;

	SRCPND = INT_IIC;
	INTPND = INT_IIC;

	if (IICSTAT & 0x8) 
		printf("\r\nBus arbitration failed during serial I/O!\r\n"); 
	
	else if (IICSTAT & 0x4) 
		printf("\r\nReceived slave address matches the address value \
			                               in the IICADD!\r\n"); 
	else if (IICSTAT & 0x2) 
		printf("\r\nReceived slave address is 00000000b!\r\n"); 
	
	else if (IICSTAT & 0x1) 
		printf("\r\nLast-received bit is 1 (ACK was not received)!\r\n"); 

	switch (i2c_mode) {
		case RD_DATA:
			if (-1 == i2c_pos) {
				i2c_pos = 0;
				if (1 == i2c_datacnt)
					IICCON = ((1<<5) | (0xf));	/* No ACK */
				else 
					IICCON = ((1<<7) | (0<<6) | (1<<5) | (0<<4) | (0xf));	/* Resume Ops */
				break;
			}
			
			i2c_buff[i2c_pos++] = IICDS;
			i2c_datacnt--;

			if (0 == i2c_datacnt) {

				IICSTAT = ((0x2<<6) | (0<<5) | (1<<4));
				IICCON = ((1<<7) | (0<<6) | (1<<5) | (0<<4) | (0xf));	/* Resume Ops */
				delay(10);
				break;
			}else {
				if (1 == i2c_datacnt)
					IICCON = ((1<<5) | (0xf));	/* No ACK */
				else
					IICCON = ((1<<7) | (0<<6) | (1<<5) | (0<<4) | (0xf));	/* Resume Ops */
				break;
			}
			

		case WR_DATA:
			if ( 0 == (i2c_datacnt--)) {
				IICSTAT = ((0x3<<6) | (0<<5) | (1<<4));		/* MastTx, STOP */
				IICCON = ((1<<7) | (0<<6) | (1<<5) | (0<<4) | (0xf));	/* Resume Ops */
				delay(10);
				break;
			}
			
			IICDS = i2c_buff[i2c_pos++];
			
			for (i = 0; i < 10; ++i);
			
			IICCON = ((1<<7) | (0<<6) | (1<<5) | (0<<4) | (0xf));	/* Resume Ops */
			break;
			
		default:
			break;
	}
}

