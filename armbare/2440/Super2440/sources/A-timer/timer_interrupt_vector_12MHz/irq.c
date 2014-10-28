
#include "s3c2440.h"
#include "irq.h"


void IRQ_ISR_ENTRY(void)
{
    unsigned long offset = INTOFFSET;

    SRCPND = 1<<offset;
    INTPND = 1<<offset;

    irq_vector[offset]();
}


