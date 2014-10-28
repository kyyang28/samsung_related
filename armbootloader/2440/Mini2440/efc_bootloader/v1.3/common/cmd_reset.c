

#include <stdio.h>

extern void delay (unsigned long ticks);
extern int disable_interrupts (void);
extern void reset_cpu(void);

int do_reset(int argc, char *argv[])
{
    printf("resetting cpu ...\r\n");

    delay(50000);

    disable_interrupts();
    reset_cpu();
    
	/* NOTREACHED */
    return 0;
}


