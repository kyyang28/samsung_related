

#include "key.h"

void key_init(void)
{
	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *	
	 *	GPF1  [3:2]  		00 = Input           01 = Output 
	 *				10 = EINT[1]         11 = Reserved
	 *
	 *	Key 2 
	 */
	GPFCON &= ~(1<<2 | 1<<3);

	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *	
	 *	GPF3  [7:6]  	00 = Input               01 = Output 
	 *			10 = EINT[3]             11 = Reserved
	 *
	 *	Key 3 
	 */
	GPFCON &= ~(1<<6 | 1<<7);

	
	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF5  [11:10]  	00 = Input               01 = Output 
	 *			10 = EINT[5]             11 = Reserved 
	 *
	 *	Key 4 
	 */
	GPFCON &= ~(1<<10 | 1<<11);


	/*
	 *	GPFCON  0x56000050  R/W  Configures the pins of port F 
	 *
	 *	GPF0  [1:0]  	00 = Input               01 = Output 
	 *			10 = EINT[0]             11 = Reserved 
	 *
	 *	Key 5 
	 */
	GPFCON &= ~(1<<0 | 1<<1);
}




