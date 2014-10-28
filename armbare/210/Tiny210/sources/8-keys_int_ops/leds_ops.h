#ifndef __LEDS_OPS_H
#define __LEDS_OPS_H

int leds_init_ll(void);
void leds_on(int num);
void leds_off(int num);
void leds_run(void);
void leds_all_on(void);
void leds_all_off(void);

#endif

