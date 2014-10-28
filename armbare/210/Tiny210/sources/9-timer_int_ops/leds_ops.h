#ifndef __LEDS_OPS_H
#define __LEDS_OPS_H

#define GPJ2CON							(*(volatile unsigned long *)0xE0200280)
#define GPJ2DAT							(*(volatile unsigned long *)0xE0200284)

int leds_init_ll(void);
void leds_on(int num);
void leds_off(int num);
void leds_run(void);
void leds_all_on(void);
void leds_all_off(void);
void delay(void);

#endif

