
#ifndef __IRQ_H__
#define __IRQ_H__

void (*isr_vector[32])(void);

static inline void dummy_ISR(void) { while(1); }


#endif


