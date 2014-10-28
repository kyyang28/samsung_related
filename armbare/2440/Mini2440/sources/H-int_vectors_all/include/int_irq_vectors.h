#ifndef __INT_IRQ_VECTORS_H
#define __INT_IRQ_VECTORS_H

void (*isr[60])(void);

int int_irq_init_ll(void);

static inline void dummyISR(void)
{
	while (1);
}

#endif

