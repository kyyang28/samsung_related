
#include "mini2440_regs.h"

#define PCLK                (50000000)
#define UART_CLK            (PCLK)        
#define UART_BAUD_RATE      (115200)
#define UART_BRD            ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)

#define TXD0READY           (1<<2)
#define RXD0READY           (1)


int serial_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3 is set to TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3 pull up

    ULCON0  = 0x03;     // 8 N 1
    UCON0   = 0x05;     // POLL Mode£¬UART clk source is PCLK
    UFCON0  = 0x00;     // No FIFO
    UMCON0  = 0x00;     // No flow control
    UBRDIV0 = UART_BRD; // baudrate is 115200
    return 0;
}

void serial_putc(unsigned char c)
{
    /* Wait untill the buffer data is sent completely */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* Write data to UTXH0, then UART sends the data automatically */
    UTXH0 = c;
}

unsigned char serial_getc(void)
{
    /* Wait until the buffer received a data */
    while (!(UTRSTAT0 & RXD0READY));
    
    /* Read the URXH0 register to obtain the data */
    return URXH0;
}

void serial_puts(char *s)
{
    while (*s != '\0')
        serial_putc(*s++);
}

int serial_tstc(void)
{
    return UTRSTAT0 & 0x1;
}

