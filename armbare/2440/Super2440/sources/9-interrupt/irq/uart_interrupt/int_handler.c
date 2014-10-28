
#include "s3c2440.h"

void IRQ_Handle(void)
{
	unsigned long offset = INTOFFSET;
	
	switch(offset)
	{
		/* INT_UART2 */
		case 15:
			UTXH2_L = '^';
			break;
			
		/* INT_UART1 */
		case 23:
			UTXH1_L = '^';
			break;
			
		/* INT_UART0 */
		case 28:
			UTXH0_L = '^';
			break;
			
		default:
			break;
	}
	
	SRCPND = 1<<offset;
	SUBSRCPND = (1<<0 | 1<<1 | 1<<3 | 1<<4 | 1<<6 | 1<<7);
	INTPND = 1<<offset;
}

