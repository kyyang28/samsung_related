#include "lib.h"

#define GPH0CON		(*(volatile unsigned int *)0xE0200C00)
#define GPH0DAT		(*(volatile unsigned int *)0xE0200C04)

void led_init(void)
{
	/* ÅäÖÃGPH0_0,GPH0_1,GPH0_2,GPH0_3ÎªÊä³öÒý½Å */
	GPH0CON = 0x1111;
}

void led_water(void)
{
	int i = 0;
	while (1)
	{		
		GPH0DAT = i;
		i++;
		if (i == 16)
			i = 0;
		delay();
	}
}

