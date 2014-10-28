
#ifndef __MINI2440_REGS
#define __MINI2440_REGS

/* Nandflash Controller Registers */
#define NFCONF                  (*(volatile unsigned long *)0x4E000000)
#define NFCONT                  (*(volatile unsigned long *)0x4E000004)
#define NFCMD                   (*(volatile unsigned char *)0x4E000008)
#define NFADDR                  (*(volatile unsigned char *)0x4E00000C)
#define NFDATA                  (*(volatile unsigned char *)0x4E000010)
#define NFSTAT                  (*(volatile unsigned long *)0x4E000020)


/* UART0 registers */
#define ULCON0                  (*(volatile unsigned long *)0x50000000)
#define UCON0                   (*(volatile unsigned long *)0x50000004)
#define UFCON0                  (*(volatile unsigned long *)0x50000008)
#define UMCON0                  (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0                (*(volatile unsigned long *)0x50000010)
#define UTXH0                   (*(volatile unsigned char *)0x50000020)
#define URXH0                   (*(volatile unsigned char *)0x50000024)
#define UBRDIV0                 (*(volatile unsigned long *)0x50000028)

/* GPIO for UART0 */
#define GPHCON                  (*(volatile unsigned long *)0x56000070)
#define GPHDAT                  (*(volatile unsigned long *)0x56000074)
#define GPHUP                   (*(volatile unsigned long *)0x56000078)


#endif

