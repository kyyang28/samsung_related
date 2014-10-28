
#include "stdio.h"
#include "interrupt.h"
#include "wdt_ops.h"
#include "leds_ops.h"

#define WTCON						(*(volatile unsigned long *)0xE2700000)
#define WTDAT						(*(volatile unsigned long *)0xE2700004)
#define WTCNT						(*(volatile unsigned long *)0xE2700008)
#define WTCLRINT					(*(volatile unsigned long *)0xE270000C)

#define PRESCALER_VAL				(99)
#define WDT_EN						(1)
#define CLOCK_DIV_FACTOR			(0)
#define INTERRUPT_EN				(1)
#define RESET_EN					(1)
#define WTDAT_VAL					(5)
#define WTCNT_VAL					(5)

#if 0
typedef unsigned long				ul;

static void wdt_ops(ul reset, ul interrupt, ul clock_div_factor, ul wdt_en, ul preval, ul wtdat, ul wtcnt)
{
	WTDAT = wtdat;
	WTCNT = wtcnt;
	
	WTCON = (preval<<8 | wdt_en<<5 | clock_div_factor<<3 | interrupt<<2 | reset<<0);
}
#endif

int wdt_init_ll(void)
{
	//wdt_ops(0, INTERRUPT_EN, CLOCK_DIV_FACTOR, WDT_EN, PRESCALER_VAL, WTDAT_VAL, WTCNT_VAL);
	WTDAT = WTDAT_VAL;
	WTCNT = WTCNT_VAL;
	
	WTCON = (PRESCALER_VAL<<8 | WDT_EN<<5 | CLOCK_DIV_FACTOR<<3 | INTERRUPT_EN<<2 | ~RESET_EN<<0);
	return 0;
}

void wdt_irq_handler(void)
{
	static int cnt = 0;
	
	/* Clear WDT interrupt */
	WTCLRINT = 1;

	printf("%d\r\n", ++cnt);

	leds_on(1);
	delay();
	leds_off(1);
	delay();

	clearVecAddress();

	if (cnt == 10) {
		printf("Resetting cpu...\r\n");
		//wdt_ops(RESET_EN, INTERRUPT_EN, CLOCK_DIV_FACTOR, WDT_EN, PRESCALER_VAL, WTDAT_VAL, WTCNT_VAL);
		WTDAT = WTDAT_VAL;
		WTCNT = WTCNT_VAL;
		WTCON = (PRESCALER_VAL<<8 | WDT_EN<<5 | CLOCK_DIV_FACTOR<<3 | INTERRUPT_EN<<2 | RESET_EN<<0);
	}
}

