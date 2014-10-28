#ifndef __ADC_OPS_H
#define __ADC_OPS_H

#define TSADCCON0							(*(volatile unsigned long *)0xE1700000)
#define TSCON0								(*(volatile unsigned long *)0xE1700004)
#define TSDLY0								(*(volatile unsigned long *)0xE1700008)
#define TSDATX0								(*(volatile unsigned long *)0xE170000C)
#define TSDATY0								(*(volatile unsigned long *)0xE1700010)
#define TSPENSTAT0							(*(volatile unsigned long *)0xE1700014)
#define CLRINTADC0							(*(volatile unsigned long *)0xE1700018)
#define ADCMUX								(*(volatile unsigned long *)0xE170001C)
#define CLRINTPEN0							(*(volatile unsigned long *)0xE1700020)

void adc_test_ll(void);

#endif

