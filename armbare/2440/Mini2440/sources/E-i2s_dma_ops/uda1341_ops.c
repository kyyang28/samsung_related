
#include "s3c2440-regs.h"
#include "L3_ops.h"

int uda1341_init_ll(void)
{
	/* UDA1341 init */
	L3_address_mode(0x14 + 2);	// 000101xx + 10 (STATUS CONTROL)
	L3_data_transfer_mode(0x50);	// 0,1,01,000,0 : 1-reset, 384fs
	DELAY(0x100);
	
	L3_address_mode(0x14 + 0);	// 000101xx + 00 (DATA0 CONTROL)
	L3_data_transfer_mode(0x02);	// -1 db (volume)
	DELAY(0x100);
	return 0;
}

