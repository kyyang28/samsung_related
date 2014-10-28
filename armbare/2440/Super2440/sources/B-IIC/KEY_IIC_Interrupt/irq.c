
#include "s3c2440.h"


extern void do_k2(void);
extern void do_k3(void);
extern void do_k4(void);
extern void do_k5(void);
extern void do_k6(void);
extern void do_k7(void);
extern void do_IIC_ISR(void);


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
        
	/* Key buttons interrupt service routine */
	case EINT0_OFFSET:  // 0
	    do_k5();
	    break;
        
	case EINT1_OFFSET:  // 1
	    do_k2();
	    break;
	
	case EINT2_OFFSET:  // 2
	    do_k6();
	    break;
	
	case EINT3_OFFSET:  // 3
	    do_k3();
	    break;
	
	case EINT4_OFFSET:  // 4
            if(EINTPEND & (1<<4))
                do_k7();
            if(EINTPEND & (1<<5))
                do_k4();
	    break;

        /* IIC interrupt service routine */
        case IIC_OFFSET:    // 27
            do_IIC_ISR();
	    break;
	
	default:
	    break;
    }

    if(offset == EINT4_OFFSET)
        EINTPEND = (1<<4 | 1<<5);

    SRCPND = 1<<offset;
    INTPND = 1<<offset;
}


