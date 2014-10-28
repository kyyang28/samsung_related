
#include <stdio.h>
#include "../include/mini2440_regs.h"

#if 0
#define GPG0_INPUT                  (0<<0) 
#define GPG3_INPUT                  (0<<6) 
#define GPG5_INPUT                  (0<<10) 
#define GPG6_INPUT                  (0<<12) 
#define GPG7_INPUT                  (0<<14) 
#define GPG11_INPUT                 (0<<22) 
#endif

extern int tstc(void);
extern unsigned char serial_getc(void);
extern void displayHWTestMenuUsage(void);
#if 0
extern void led_init(void);
extern void led_on(int num);
extern void led_off(int num);
extern void led_all_on(void);
extern void led_all_off(void);
#endif

void leds_init(void)
{
    GPBCON |= (1<<10) | (1<<12) | (1<<14) | (1<<16);
}

void leds_on(int num)
{
    switch (num) {
        case 1:
        case 2:
        case 3:
        case 4:
            GPBDAT = (~(1<<(num+4)));
            break;
        default:
            printf("\r\nInvalid LED number, please try 1,2,3,4!\r\n");
            break;
    }
}

void leds_off(int num)
{
    switch (num) {
        case 1:
        case 2:
        case 3:
        case 4:
            GPBDAT = (1<<(num+4));
            break;
        default:
            printf("\r\nInvalid LED number, please try 1,2,3,4!\r\n");
            break;
    }
}

void leds_all_on(void)
{
    GPBDAT = ~(1<<5 | 1<<6 | 1<<7 | 1<<8);
}

void leds_all_off(void)
{
    printf("leds_all_off");
    GPBDAT = (1<<5 | 1<<6 | 1<<7 | 1<<8);
}


void buttons_init(void)
{
    //GPGCON = GPG0_INPUT | GPG3_INPUT | GPG5_INPUT | GPG6_INPUT | GPG7_INPUT | GPG11_INPUT;
    GPGCON = (0<<0) | (0<<6) | (0<<10) | (0<<12) | (0<<14) | (0<<22);
}

void buttons_led(void)
{
    unsigned long dwDat;

    while(1) {

        dwDat = GPGDAT;
        
        if (dwDat & (1<<0))
            leds_off(1);
        else {    
            printf("K1");
            leds_on(1);
        }
        
        if (dwDat & (1<<3))
            leds_off(2);
        else {
            printf("K2");
            leds_on(2);
        }
        
    	if (dwDat & (1<<5)) 
            leds_off(3);
    	else	
            leds_on(3);
    		
    	if (dwDat & (1<<6)) 
            leds_off(4);
    	else	
            leds_on(4);

    	if (dwDat & (1<<7)) {
            leds_off(1);
            leds_off(2);
    	}else {	
            leds_on(1);
            leds_on(2);
    	}

        if (dwDat & (1<<11)) {
            //leds_all_off();
        }else { 
            leds_all_on();
        }        
    }
}

void buttons_test(void)
{
    leds_init();    
    leds_all_off();
    buttons_init();    
    buttons_led();
}

