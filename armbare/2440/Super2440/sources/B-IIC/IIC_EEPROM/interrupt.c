
#include "s3c2440.h"
#include "interrupt.h"


void IRQ_ISR_ENTRY(void)
{
    ISR_VECTOR[INTOFFSET]();    
}


