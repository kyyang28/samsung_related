
#ifndef __S3C2440_H__
#define __S3C2440_H__

/* Watchdog */
#define	WTCON				(*(volatile unsigned long *)0x53000000)

/* LED */
#define GPFCON                          (*(volatile unsigned long *)0x56000050)
#define GPFDAT                          (*(volatile unsigned long *)0x56000054)

#define GPGCON                          (*(volatile unsigned int *)0x56000060)
#define GPGDAT                          (*(volatile unsigned int *)0x56000064)

#define	S3C2440_CLOCK_BASE		0x4c000000
#define MDIV				0x5c
#define PDIV				0x01
#define SDIV				0x01

typedef unsigned int	u32;

typedef volatile u32	S3C2440_REG32;

typedef struct {
	S3C2440_REG32 LOCKTIME;
	S3C2440_REG32 MPLLCON;
	S3C2440_REG32 UPLLCON;
	S3C2440_REG32 CLKCON;
	S3C2440_REG32 CLKSLOW;
	S3C2440_REG32 CLKDIVN;
	S3C2440_REG32 CAMDIVN;
}/*__attribute__((__packed__)) */S3C2440_CLOCK;


static inline S3C2440_CLOCK * const S3C2440_Get_CLOCK_Base(void)
{
	return (S3C2440_CLOCK * const)S3C2440_CLOCK_BASE;
}

void led_init(void);
void key_init(void);
void clock_init(void);

#endif

