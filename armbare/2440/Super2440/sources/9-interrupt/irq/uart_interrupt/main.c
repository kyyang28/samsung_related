
#include "uart.h"

#define COM0		(0)

static inline void DELAY (unsigned long ticks)
{
	__asm__ volatile (
	  "1:\n"
	  "subs %0, %1, #1\n"
	  "bne 1b":"=r" (ticks):"0" (ticks));
}


int mymain(void)
{
	char alphabet[27] = "abcdefghijklmnopqrstuvwxyz";
	
	int i;
	while(1) {
		for(i = 0; i < 26; ++i) {
			uart_putchar(COM0, alphabet[i]);
			DELAY(100000);
		}		
	}	
}

