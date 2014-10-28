
#include "s3c2440-regs.h"

int i2s_init_ll(void)
{
	// GPE0,1,2,3,4 = IIS signal
	GPECON |= 1<<1 | 1<<3 | 1<<5 | 1<<7 | 1<<9;
	GPECON &= ~(1<<0 | 1<<2 | 1<<4 | 1<<6 | 1<<8);	

	// IIS interface [0] 	0 = Disable (stop)
	// 			1 = Enable  (start)
	IISCON |= 1<<0;
	
	// IIS prescaler [1] 	0 = Disable
	// 			1 = Enable
	IISCON |= 1<<1;
	
	// Transmit/receive mode select : Transmit and receive
	// [7:6] 00 = No transfer          01 = Receive mode
	// 	 10 = Transmit mode    11 = Transmit and receive mod
	IISMOD |= 1<<6 | 1<<7;
	
	// Serial data bit per channel : 16-bit
	// [3] 0 = 8-bit         1 = 16-bit
	IISMOD |= 1<<3;
	
	// Master clock frequency select : 384fs
	// [2] 0 = 256fs        1 = 384fs
	//	(fs: sampling frequency)
	IISMOD |= 1<<2;
	
	// Serial bit clock frequency select : 32fs
	// [1:0] 00 = 16fs        01 = 32fs
	//	10 = 48fs        11 = N/A
	IISMOD |= 1<<0;
	
	//  Prescaler B makes the master clock that is used the external
    //   block and division factor is N+1
	// N + 1 = 50 * 1000000 / (384 * 44100) = 2.959
	// N = 2;
	// N + 1 = 50 * 1000000 / (384 * 22050) = 5.9
	// N = 5	
	IISPSR = 5<<0 | 5<<5;
		
	// Transmit FIFO [13] 0 = Disable     1 = Enable
	IISFCON |= 1<<13;
	
	// Receive FIFO [12] 0 = Disable     1 = Enable
	IISFCON |= 1<<12;	

	return 0;
}

void i2s_write_ll(short * pdata, int data_size)
{		
	// write wav data at address	
	while(data_size > 0) {
		while((IISCON & (1<<7)) == (1<<7));
		
		// IISFIFO <-- Wav Audio Data
		IISFIFO = *pdata++;
		data_size -= 2;
	}
}	

void i2s_read_ll(short * pdata, int data_size)
{		
	// read wav data at address	
	while(data_size > 0) {
		while((IISCON & (1<<6)) == (1<<6));
		
		// Wav Audio Data <--- IISFIFO
		*pdata++ = IISFIFO;
		data_size -= 2;
	}
}

