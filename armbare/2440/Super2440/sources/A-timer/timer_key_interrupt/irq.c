
#include "s3c2440.h"


extern void do_k2(void);
extern void do_k3(void);
extern void do_k4(void);
extern void do_k5(void);
extern void do_k6(void);
extern void do_k7(void);
extern void do_timer0(void);


void IRQ_ISR_ENTRY(void)
{
    unsigned long offset = INTOFFSET;

    /* 
     * EINT1(K2), EINT3(K3), EINT5(K4)
     * lights on led 1, led 2, led 3 respectively
     *
     * EINT0(K5), EINT2(K6), EINT4(K7)
     * lights off led 1, led 2, led 3 respectively
     */
    switch(offset) {
        
	case 0:
	    do_k5();
	    break;
        
	case 1:
	    do_k2();
	    break;
	
	case 2:
	    do_k6();
	    break;
	
	case 3:
	    do_k3();
	    break;
	
	case 4:
            if(EINTPEND & (1<<4))
                do_k7();
            if(EINTPEND & (1<<5))
                do_k4();
            break;

        case 10:
	    do_timer0();
	    break;

	default:
	    break;
    }

    if(offset == 4)
        EINTPEND = (1<<4 | 1<<5);

    SRCPND = 1<<offset;
    INTPND = 1<<offset;
}


