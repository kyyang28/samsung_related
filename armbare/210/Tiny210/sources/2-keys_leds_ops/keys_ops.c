
#include "leds_ops.h"

/* GPIO for K1, K2, K3, K4 */
#define GPH2CON							(*(volatile unsigned long *)0xE0200C40)
#define GPH2DAT							(*(volatile unsigned long *)0xE0200C44)

/* GPIO for K5, K6, K7, K8 */
#define GPH3CON							(*(volatile unsigned long *)0xE0200C60)
#define GPH3DAT							(*(volatile unsigned long *)0xE0200C64)

int keys_init(void)
{
	/* K1, K2, K3, K4 init */
	GPH2CON = 0x0<<12 | 0x0<<8 | 0x0<<4 | 0x0<<0;	/* GPH2CON = 0x0 */

	/* K5, K6, K7, K8 init */
	GPH3CON = 0x0<<12 | 0x0<<8 | 0x0<<4 | 0x0<<0;	/* GPH3CON = 0x0 */

	return 0;
}

static void k1_test(void)
{
	if (GPH2DAT & (1<<0))
		leds_off(1);
	else
		leds_on(1);
}

static void k2_test(void)
{
	if (GPH2DAT & (1<<1))
		leds_off(2);
	else
		leds_on(2);
}

static void k3_test(void)
{
	if (GPH2DAT & (1<<2))
		leds_off(3);
	else
		leds_on(3);
}

static void k4_test(void)
{
	if (GPH2DAT & (1<<3))
		leds_off(4);
	else
		leds_on(4);
}

static void k5_test(void)
{
	if (GPH3DAT & (1<<0)) {
		leds_off(1);
		leds_off(3);
	}else {
		leds_on(1);
		leds_on(3);
	}
}

static void k6_test(void)
{
	if (GPH3DAT & (1<<1)) {
		leds_off(2);
		leds_off(4);
	}else {
		leds_on(2);
		leds_on(4);
	}
}

static void k7_test(void)
{
	if (GPH3DAT & (1<<2)) {
		leds_off(1);
		leds_off(2);
	}else {
		leds_on(1);
		leds_on(2);
	}
}

static void k8_test(void)
{
	if (GPH3DAT & (1<<3)) {
		leds_off(3);
		leds_off(4);
	}else {
		leds_on(3);
		leds_on(4);
	}
}

void keys_leds_test(void)
{
	while (1) {
		/* K1 test */
		k1_test();

		/* K2 test */
		k2_test();

		/* K3 test */
		k3_test();

		/* K4 test */
		k4_test();

		/* K5 test */
		k5_test();

		/* K6 test */
		k6_test();

		/* K7 test */
		k7_test();

		/* K8 test */
		k8_test();
	}
}

