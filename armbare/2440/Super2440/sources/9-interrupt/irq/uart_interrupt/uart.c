
#include "s3c2440.h"


/* Transmit a character */
void uart_putchar(int COM, unsigned char ch)
{
	if(COM == 0) {
		while(!(UTRSTAT0 & TX_READY));
		UTXH0_L = ch;
	}else if(COM == 1) {
		while(!(UTRSTAT1 & TX_READY));
		UTXH1_L = ch;
	}else if(COM == 2) {
		while(!(UTRSTAT2 & TX_READY));
		UTXH2_L = ch;
	}
}

