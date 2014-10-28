
#include "s3c2440-regs.h"

int ts_init_ll(void)
{
	ADCCON = (1<<14) | (49<<6);
	ADCDLY = 50000;
	return 0;
}

