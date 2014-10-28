
#include "s3c2440-regs.h"
#include "int_irq_vectors.h"
#include "leds_ops.h"
#include "buzzer_ops.h"
#include "uart_ops.h"
#include "rtc_ops.h"
#include "ts_ops.h"
#include "i2c_ops.h"
#include "eeprom_ops.h"
#include "printf.h"

typedef int (init_func)(void);

init_func *init_func_pa[] = {
	leds_init_ll,
	buzzer_init_ll,
	uart_init_ll,
	ts_init_ll,
	i2c_init_ll,
	int_irq_init_ll,
	NULL,
};

int int_vector_all_main(void)
{
	init_func **init_func_pp;

	for (init_func_pp = init_func_pa; *init_func_pp; ++init_func_pp) {
		if ((*init_func_pp)() != 0) 
			for (;;);
	}

	/* RTC operations */
	rtc_setup_time(12, 10, 12, 12, 28, 26);
	rtc_alarm_setup_time(12, 10, 12, 12, 28, 32);

	while (1) {
		/* RTC display routine */
		rtc_display_time();

		/* I2c eeprom test routine */
		i2c_eeprom_test();
	}

	return 0;
}

