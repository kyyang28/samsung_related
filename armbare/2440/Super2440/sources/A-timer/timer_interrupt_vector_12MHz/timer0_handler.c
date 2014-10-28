
#include "s3c2440.h"


void do_timer0(void)
{
    GPFDAT = ~(GPFDAT);
    GPGDAT = ~(GPGDAT);
}


