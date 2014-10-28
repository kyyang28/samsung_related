
#ifndef __KEY_H__
#define __KEY_H__

#define GPFCON				(*(volatile unsigned long *)0x56000050)
#define GPFDAT				(*(volatile unsigned long *)0x56000054)

#define GPGCON                  	(*(volatile unsigned int *)0x56000060)
#define GPGDAT                      	(*(volatile unsigned int *)0x56000064)

void key_init(void);

#endif

