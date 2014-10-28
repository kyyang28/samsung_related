
#ifndef __S3C2440_H__
#define __S3C2440_H__


/* Types */
typedef unsigned char			u8;
typedef unsigned int			u32;
typedef volatile u8				S3C2440_REG8;
typedef volatile u32			S3C2440_REG32;	

/* Watchdog */
#define	WTCON				(*(volatile unsigned long *)0x53000000)

/* LED */
#define GPFCON                          (*(volatile unsigned long *)0x56000050)
#define GPFDAT                          (*(volatile unsigned long *)0x56000054)

#define GPGCON                          (*(volatile unsigned long *)0x56000060)
#define GPGDAT                          (*(volatile unsigned long *)0x56000064)

/* Interrupt */
#define SRCPND                          (*(volatile unsigned long *)0x4A000000)
#define INTMOD                          (*(volatile unsigned long *)0x4A000004)
#define INTMSK                          (*(volatile unsigned long *)0x4A000008)
#define PRIORITY                        (*(volatile unsigned long *)0x4A00000C)
#define INTPND                          (*(volatile unsigned long *)0x4A000010)
#define INTOFFSET                       (*(volatile unsigned long *)0x4A000014)
#define SUBSRCPND                       (*(volatile unsigned long *)0x4A000018)
#define INTSUBMSK                       (*(volatile unsigned long *)0x4A00001C)

/* External Interrupt */
#define EINTMASK                        (*(volatile unsigned long *)0x560000A4)
#define EINTPEND                        (*(volatile unsigned long *)0x560000A8)


/* GPHCON for UART0,1,2  */
#define GPHCON				(*(S3C2440_REG32 *)0x56000070)
#define GPHDAT				(*(S3C2440_REG32 *)0x56000074)
#define GPHUP					(*(S3C2440_REG32 *)0x56000078)


/* UART Registers */
/* ULCONx */
#define ULCON0				(*(S3C2440_REG32 *)0x50000000)
#define ULCON1				(*(S3C2440_REG32 *)0x50004000)
#define ULCON2				(*(S3C2440_REG32 *)0x50008000)

/* UCONx */
#define UCON0				(*(S3C2440_REG32 *)0x50000004)
#define UCON1				(*(S3C2440_REG32 *)0x50004004)
#define UCON2				(*(S3C2440_REG32 *)0x50008004)

/* UFCONx */
#define UFCON0				(*(S3C2440_REG32 *)0x50000008)
#define UFCON1				(*(S3C2440_REG32 *)0x50004008)
#define UFCON2				(*(S3C2440_REG32 *)0x50008008)

/* UMCONx */
#define UMCON0				(*(S3C2440_REG32 *)0x5000000C)
#define UMCON1				(*(S3C2440_REG32 *)0x5000400C)

/* UTRSTATx */
#define UTRSTAT0			(*(S3C2440_REG32 *)0x50000010)
#define UTRSTAT1			(*(S3C2440_REG32 *)0x50004010)
#define UTRSTAT2			(*(S3C2440_REG32 *)0x50008010)

/* UTXHx Little endian */
#define UTXH0_L				(*(S3C2440_REG8 *)0x50000020)	
#define UTXH1_L				(*(S3C2440_REG8 *)0x50004020)	
#define UTXH2_L				(*(S3C2440_REG8 *)0x50008020)	

/* URXHx Little endian */
#define URXH0_L				(*(S3C2440_REG8 *)0x50000024)	
#define URXH1_L				(*(S3C2440_REG8 *)0x50004024)	
#define URXH2_L				(*(S3C2440_REG8 *)0x50008024)	

/* UBRDIVx */
#define UBRDIV0				(*(S3C2440_REG32 *)0x50000028)
#define UBRDIV1				(*(S3C2440_REG32 *)0x50004028)
#define UBRDIV2				(*(S3C2440_REG32 *)0x50008028)


/* UART */
#define PCLK				(12000000)
#define UART_CLOCK			(PCLK)
#define UART_BAUDRATE			(57600)

#define TX_READY			(1<<2)
#define RX_READY			(1)

#endif

