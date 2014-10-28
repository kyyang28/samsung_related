
#ifndef __S3C2440_H__
#define __S3C2440_H__


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


/* Timer0 */
#define TCFG0                           (*(volatile unsigned long *)0x51000000)
#define TCFG1                           (*(volatile unsigned long *)0x51000004)
#define TCON                            (*(volatile unsigned long *)0x51000008)
#define TCNTB0                          (*(volatile unsigned long *)0x5100000C)
#define TCMPB0                          (*(volatile unsigned long *)0x51000010)
#define TCNTO0                          (*(volatile unsigned long *)0x51000014)


/* Interrupt offset numbers */
#define TIMER0_OFT                      (10)


#endif

