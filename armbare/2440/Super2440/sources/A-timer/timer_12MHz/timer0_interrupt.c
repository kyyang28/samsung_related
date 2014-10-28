
#include "s3c2440.h"
#include "led.h"

void timer0_ISR(void)
{
    if(INTOFFSET == 10)
        LedRun();

    SRCPND = 1<<INTOFFSET;
    INTPND = 1<<INTOFFSET;
}


