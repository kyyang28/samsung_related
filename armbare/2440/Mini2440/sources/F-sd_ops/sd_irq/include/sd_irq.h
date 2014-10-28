#ifndef __SD_IRQ_H
#define __SD_IRQ_H

void (*isr[60])(void);

void sd_int_init(void);
void sd_isr(void);

#endif

