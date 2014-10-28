
#include "s3c2440.h"
#include "irq.h"

void IRQ_ISR_ENTRY(void)
{
    unsigned long offset = INTOFFSET;

    if(offset == 4)
        EINTPEND = (1<<4 | 1<<5);
    
    SRCPND = 1<<offset;
    INTPND = 1<<offset;

    isr_vector[offset]();
}


