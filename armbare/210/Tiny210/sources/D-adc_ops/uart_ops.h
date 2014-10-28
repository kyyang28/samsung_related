#ifndef __UART_OPS_H
#define __UART_OPS_H

/* GPIO for Uart0 */
#define GPA0CON								(*(volatile unsigned long *)0xE0200000)
#define GPA0DAT								(*(volatile unsigned long *)0xE0200004)

/* GPIO for Uart3 */
#define GPA1CON								(*(volatile unsigned long *)0xE0200020)
#define GPA1DAT								(*(volatile unsigned long *)0xE0200024)

/* Uart0 Controller Registers */
#define ULCON0								(*(volatile unsigned long *)0xE2900000)
#define UCON0								(*(volatile unsigned long *)0xE2900004)
#define UFCON0								(*(volatile unsigned long *)0xE2900008)
#define UTRSTAT0							(*(volatile unsigned long *)0xE2900010)
#define UFSTAT0								(*(volatile unsigned long *)0xE2900018)
#define UTXH0								(*(volatile unsigned char *)0xE2900020)
#define URXH0								(*(volatile unsigned char *)0xE2900024)
#define UBRDIV0								(*(volatile unsigned long *)0xE2900028)
#define UDIVSLOT0							(*(volatile unsigned long *)0xE290002C)

/* Uart3 Controller Registers */
#define ULCON3								(*(volatile unsigned long *)0xE2900C00)
#define UCON3								(*(volatile unsigned long *)0xE2900C04)
#define UFCON3								(*(volatile unsigned long *)0xE2900C08)
#define UTRSTAT3							(*(volatile unsigned long *)0xE2900C10)
#define UFSTAT3								(*(volatile unsigned long *)0xE2900C18)
#define UTXH3								(*(volatile unsigned char *)0xE2900C20)
#define URXH3								(*(volatile unsigned char *)0xE2900C24)
#define UBRDIV3								(*(volatile unsigned long *)0xE2900C28)
#define UDIVSLOT3							(*(volatile unsigned long *)0xE2900C2C)

#define PCLK								(66000000)
#define BAUD_RATE							(115200)
#define UBRDIV0_VAL							((PCLK / (BAUD_RATE * 16)) - 1)

int uart_init_ll(void);
void uart_putc(unsigned char ch);
unsigned char uart_getc(void);
void uart_puts(char *str);
void uart_test_ll(void);

#endif

