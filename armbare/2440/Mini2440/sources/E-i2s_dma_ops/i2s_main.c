
#include "s3c2440-regs.h"
#include "nand_ops.h"
#include "leds_ops.h"
#include "L3_ops.h"
#include "i2s_ops.h"
#include "uda1341_ops.h"
#include "audiodrv_ops.h"
#include "cache_ops.h"

typedef int (init_func)(void);

init_func *init_func_pa[] = {
	leds_init_ll,
	L3_init_ll,
	i2s_init_ll,
	uda1341_init_ll,
#ifdef ICACHE_ENABLED
	icache_enable,
#endif
	NULL,
};

int i2s_main(void)
{
	init_func **init_func_pp;

	/* Hardware initializations */
	for (init_func_pp = init_func_pa; *init_func_pp; ++init_func_pp) {
		if ((*init_func_pp)() != 0)
			for (;;);
	}

	/* Load audio wav file from nandflash to sdram */
	nand_read_ll(0x20000, (unsigned char *)0x32000000, 0x100000);

	/* Play the wav sound */
	play_wavfile(0x32000000);

	while(1)
		leds_ops();
	
#if 0
	record_wavdata(0x33000000, 500000);
	beep_once();
	
	IIS_write((short *)0x33000000, 500000);
	
	beep_once();
	beep_once();
#endif

	while(1);

	return 0;
}

