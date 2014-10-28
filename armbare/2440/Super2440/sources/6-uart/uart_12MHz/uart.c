
#include "s3c2440.h"


/* UART initialization */
void uart_init(int COM)
{
	/* Configurate the GPHCON of the UART0, 1, 2 */
	GPHCON = 0xAAA0;

	if(COM == 0) {
		/* 
		 * 1. Word length : 8-bit
		 * 2. Stop bit : 1
		 * 3. Parity mode: no parity
		*/
		ULCON0 = 0x3;
				
		/* 
		 * 1. Clock source: PCLK 
		 * 2. Transmit mode: Polling
		 * 3. Receive mode: Polling
		 */
		UCON0 = 0x5;
				
		/* FIFO disable */
		UFCON0 = 0x0;
		
		/* Flow Control disable */
		UMCON0 = 0x0;
				
		/* Baudrate configuration */
		UBRDIV0 = ( (int)( UART_CLOCK / ( UART_BAUDRATE * 16 ) ) - 1 );
	}else if(COM == 1) {
		ULCON1 = 0x3;				
		UCON1 = 0x5;
		UFCON1 = 0x0;
		UMCON1 = 0x0;
		UBRDIV1 = ( (int)( UART_CLOCK / ( UART_BAUDRATE * 16 ) ) - 1 );
	}else if (COM == 2) {
		ULCON2 = 0x3;				
		UCON2 = 0x5;
		UFCON2 = 0x0;
		UBRDIV2 = ( (int)( UART_CLOCK / ( UART_BAUDRATE * 16 ) ) - 1 );
	}
}


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


unsigned char uart_getchar(int COM)
{
	unsigned char res;

	if(COM == 0) {
		while(!(UTRSTAT0 & RX_READY));
		res = URXH0_L;
	}else if(COM == 1) {
		while(!(UTRSTAT1 & RX_READY));
		res = URXH1_L;
	}else if(COM == 2) {
		while(!(UTRSTAT2 & RX_READY));
		res = URXH2_L;
	}

	return res;
}

