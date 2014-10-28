

#include "mini2440_regs.h"


extern void nand_read_ll(unsigned int addr, unsigned char *buff, unsigned int size);


static int isNorFlash(void)
{
    volatile unsigned long *p = (volatile unsigned long *)0;
    unsigned int tmp;
    
    tmp = *p;
    *p = 0xABCDEF;

    if (*p == 0xABCDEF) {
        /* Boot from Nandflash */
        *p = tmp;
        return 0;
    }else {
        /* Boot from Norflash */
        *p = tmp;
        return 1;
    }
}

void copy2ram(unsigned char *src, unsigned char *dest, unsigned int size)
{
    int i;
    
    if (isNorFlash()) {
        /* Boot from Norflash */
        for (i = 0; i < size; ++i)
            dest[i] = src[i];
    }else {
        /* Boot from Nandflash */
        nand_read_ll((unsigned int)src, dest, size);
    }
}


