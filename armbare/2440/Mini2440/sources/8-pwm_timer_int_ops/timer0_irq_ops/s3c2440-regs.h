#ifndef __S3C2440_REGS_H
#define __S3C2440_REGS_H

/* GPIO registers */
#define GPBCON              (*(volatile unsigned long *)0x56000010)
#define GPBDAT              (*(volatile unsigned long *)0x56000014)

/*interrupt registes*/
#define SRCPND              (*(volatile unsigned long *)0x4A000000)
#define INTMOD              (*(volatile unsigned long *)0x4A000004)
#define INTMSK              (*(volatile unsigned long *)0x4A000008)
#define PRIORITY            (*(volatile unsigned long *)0x4A00000C)
#define INTPND              (*(volatile unsigned long *)0x4A000010)
#define INTOFFSET           (*(volatile unsigned long *)0x4A000014)
#define SUBSRCPND           (*(volatile unsigned long *)0x4A000018)
#define INTSUBMSK           (*(volatile unsigned long *)0x4A00001C)


/* PWM & Timer registers */
#define	TCFG0				(*(volatile unsigned long *)0x51000000)
#define	TCFG1				(*(volatile unsigned long *)0x51000004)
#define	TCON				(*(volatile unsigned long *)0x51000008)
#define	TCNTB0				(*(volatile unsigned long *)0x5100000C)
#define	TCMPB0				(*(volatile unsigned long *)0x51000010)
#define	TCNTO0				(*(volatile unsigned long *)0x51000014)

#endif

