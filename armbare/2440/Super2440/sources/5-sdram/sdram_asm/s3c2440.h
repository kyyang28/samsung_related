
#ifndef __S3C2440_H__
#define __S3C2440_H__

/* Watchdog */
#define	WTCON				(*(volatile unsigned long *)0x53000000)

/* LED */
#define GPFCON                          (*(volatile unsigned long *)0x56000050)
#define GPFDAT                          (*(volatile unsigned long *)0x56000054)

#define GPGCON                          (*(volatile unsigned int *)0x56000060)
#define GPGDAT                          (*(volatile unsigned int *)0x56000064)


#endif

