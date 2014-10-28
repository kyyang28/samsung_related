
#ifndef __INTERRUPT_H__
#define __INTERRUPT_H__


#define NUMOFISR                (32)
void (*ISR_VECTOR[NUMOFISR])(void);

static inline void dummy_ISR(void) { while(1); }

#endif


