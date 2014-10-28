
#include "s3c2440.h"


/* Disable watchdog function */
void disable_watchdog(void)
{
	WTCON = 0;
}


/* LED init function */
void led_init(void)
{
	// GPF6 LED1  [13:12]   01 = Output
	GPFCON |= 1<<12;
	GPFCON &= ~(1<<13);
	                        
	// GPG0 LED2    [1:0]       01 = Output
	GPGCON |= 1<<0;
	GPGCON &= ~(1<<1);
	                                        
	// GPG1 LED3    [3:2]       01 = Output
	GPGCON |= 1<<2;
	GPGCON &= ~(1<<3);
	                                                        
	// GPG10 LED4   [21:20]     01 = Output
	GPGCON |= 1<<20;
	GPGCON &= ~(1<<21);
	
	// light all off
	GPFDAT |= 1<<6;
	GPGDAT |= (1<<0 | 1<<1 | 1<<10);
}


/* UART init function */
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


/* IRQ init function */
void irq_init(void)
{
	/* INT_UART0[28], INT_UART1[23], INT_UART2[15] -- IRQ Mode */
	INTMOD &= ~(1<<28 | 1<<23 | 1<<15);	
	
	/* INT_UART0[28], INT_UART1[23], INT_UART2[15] -- Service Available */
	INTMSK &= ~(1<<28 | 1<<23 | 1<<15);
	
	/* INT_SUBMASK UART0 */
	INTSUBMSK &= ~(1<<0 | 1<<1);
	/* INT_SUBMASK UART1 */
	INTSUBMSK &= ~(1<<3 | 1<<4);
	/* INT_SUBMASK UART2 */
	INTSUBMSK &= ~(1<<6 | 1<<7);
	
	/* Enable IRQ bit in the CPSR register */
	__asm__ (
		"msr	cpsr_c, #0x53\n" // SVC with IRQ[bit 7] is enabled
	);
}

