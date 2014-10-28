
#include "stdio.h"
#include "interrupt.h"
#include "timer_ops.h"
#include "leds_ops.h"

#define PRESCALER0_VAL					(49)
#define DIVIDER_MUX0					(4)
#define TIMER0_CNT_BUFFER				(83)
#define TIMER0_CMP_BUFFER				(0)
#define TIMER0_AUTO_RELOAD_ON			(1)
#define TIMER0_INVERTER_ON				(1)
#define TIMER0_MANUAL_UPDATE			(1)
#define TIMER0_START					(1)
#define TIMER0_INT_EN					(1)

#define TIMER0_INT_CLEAR_BIT			(1)

int cnt = 0;

int timer0_init_ll(void)
{
	/* Timer Input Clock Frequency = PCLK / ( {prescaler value + 1} ) / {divider value} */
	/* Setup the TCFG0 register */
	TCFG0 |= PRESCALER0_VAL<<0;
	//TCFG0 |= 65<<0;

	/* Setup the TCFG1 register */
	TCFG1 |= DIVIDER_MUX0<<0;

	/* Setup the TCNTB0 register */
	TCNTB0 = TIMER0_CNT_BUFFER;		/* 1s = 66700000 / (49+1) / 16 */

	/* Setup the TCMPB0 register */
	TCMPB0 = TIMER0_CMP_BUFFER;

	/* Setup the TCON register */
	/* Turn the inverter off bit */
	TCON &= ~(TIMER0_INVERTER_ON<<2);
	
	/* Set the manual update bit */
	TCON |= TIMER0_MANUAL_UPDATE<<1;

	/* Set auto-reload and start the timer0 */
	TCON |= (TIMER0_AUTO_RELOAD_ON<<3 | TIMER0_START<<0);

	/* Clear the timer0 manual update bit */
	TCON &= ~(TIMER0_MANUAL_UPDATE<<1);

	/* Enable Timer0 interrupt */
	TINT_CSTAT |= TIMER0_INT_EN<<0;

	return 0;
}

void timer0_irq_handler(void)
{
	printf("Entering timer0_irq_handler...\r\n");

	/* Clear Timer0 */
	TINT_CSTAT |= TIMER0_INT_CLEAR_BIT<<5;
	
	printf("Timer0 Int Counter = %d\r\n", cnt++);
	//GPJ2DAT = ~(GPJ2DAT & 0xF<<0);

	/* Clear vector address */
	clearVecAddress();
}

