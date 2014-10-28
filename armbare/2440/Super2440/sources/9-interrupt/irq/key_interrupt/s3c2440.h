
#ifndef __S3C2440_H__
#define __S3C2440_H__

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


#endif

