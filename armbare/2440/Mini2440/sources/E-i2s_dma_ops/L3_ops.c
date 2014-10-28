

#include "s3c2440-regs.h"

/* 
 *	L3 interface 
 *	GPB2 - L3_mode_ll
 *	GPB3 - L3_data_ll
 *	GPB4 - L3_clock_ll
 */
static void L3_mode_ll(int val)
{
	if (0 == val)
		GPBDAT &= ~(1<<2);
	else if (1 == val)
		GPBDAT |= (1<<2);
}

static void L3_data_ll(int val)
{
	if (0 == val)
		GPBDAT &= ~(1<<3);
	else if (1 == val)
		GPBDAT |= (1<<3);
}

static void L3_clock_ll(int val)
{
	if (0 == val)
		GPBDAT &= ~(1<<4);
	else if (1 == val)
		GPBDAT |= (1<<4);
}

int L3_init_ll(void)
{
	// GPB2,3,4 = output
	GPBCON |= (0x1<<4 | 0x1<<6 | 0x1<<8);
	//GPBCON |= 1<<4 | 1<<6 | 1<<8;
	//GPBCON &= ~(1<<5 | 1<<7 | 1<<9);
	return 0;
}	

void L3_data_shift(int data)
{
	int i;
	
	for (i = 0; i < 8; i++) {
		int bit;
		
		bit = (data & (1<<i)) ? 1 : 0;
		
		// write data 1 bit 
		// Tsu(L3)DA - L3_data_ll set-up time > 190ns	
		L3_data_ll(bit);
		
		// Tclk(L3)L - L3_clock_ll low time > 250ns
		// CLOCK 0 -> 1 : rising edge
		L3_clock_ll(0);
		DELAY(250);	
		L3_clock_ll(1);
		DELAY(250);	
	}	
}

void L3_address_mode(int data)
{	
	// Th(L3)A - L3_mode_ll hold time > 190ns
	L3_clock_ll(1);
	L3_mode_ll(1);
	DELAY(190);	
	
	// Tsu(L3)A - L3_mode_ll set-up time > 190ns
	L3_mode_ll(0);	// address mode
	DELAY(190);
	
	L3_data_shift(data);
	
	// Th(L3)A - L3_mode_ll hold time > 190ns
	L3_clock_ll(1);
	DELAY(190);
	L3_mode_ll(1);
	DELAY(190);
}

void L3_data_transfer_mode(int data)
{
	L3_mode_ll(1);	// data transfer mode
		
	// Tstp(L3)A - L3_mode_ll halt time > 190ns
	L3_mode_ll(0);	// data transfer mode
	DELAY(190);
	
	// Tsu(L3)A - L3_mode_ll set-up time > 190ns
	L3_clock_ll(1);
	L3_mode_ll(1);
	DELAY(190);
	
	L3_data_shift(data);
	
	// Th(L3)A - L3_mode_ll hold time > 190ns
	L3_clock_ll(1);
	DELAY(190);
	L3_mode_ll(0);
	DELAY(190);
	
	L3_mode_ll(1);	// data transfer mode
}

