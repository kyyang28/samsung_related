
#include "s3c2440.h"
#include "led.h"


/* EINT1 */
void do_k2(void) 
{
    led_on(1);
}


/* EINT3 */
void do_k3(void)
{
    led_on(2);
}


/* EINT5 */
void do_k4(void)
{
    led_on(3);
}


/* EINT0 */
void do_k5(void)
{
    led_off(1);
}


/* EINT2 */
void do_k6(void)
{
    led_off(2);
}


/* EINT4 */
void do_k7(void)
{
    led_off(3);
}


void do_k4_k7(void)
{
    if(EINTPEND & (1<<4))
        do_k7();

    if(EINTPEND & (1<<5))
        do_k4();
}


