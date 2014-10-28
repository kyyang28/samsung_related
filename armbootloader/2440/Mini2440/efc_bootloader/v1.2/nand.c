

#include "mini2440_regs.h"


#define NAND_PAGE_SIZE          (2048)

void nand_init_ll(void)
{
#define TACLS       0
#define TWRPH0      1
#define TWRPH1      0

    NFCONF = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
    NFCONT = (1<<4) | (1<<1) | (1<<0);
}

static void nand_chip_select_ll(void)
{
    NFCONT &= ~(1<<1);
}

static void nand_chip_deselect_ll(void)
{
    NFCONT |= (1<<1);
}

static void nand_send_cmd_ll(unsigned char cmd)
{
    volatile int i;
    NFCMD = cmd;
    for (i = 0; i < 10; ++i);
}

static void nand_send_addr_ll(unsigned int addr)
{
    volatile int i;
    unsigned int column = addr % NAND_PAGE_SIZE;
    unsigned int row = addr / NAND_PAGE_SIZE;

    NFADDR = column & 0xFF;
    for (i = 0; i < 10; ++i);

    NFADDR = (column >> 8) & 0xFF;
    for (i = 0; i < 10; ++i);

    NFADDR = row & 0xFF;
    for (i = 0; i < 10; ++i);

    NFADDR = (row >> 8) & 0xFF;
    for (i = 0; i < 10; ++i);

    NFADDR = (row >> 16) & 0xFF;
    for (i = 0; i < 10; ++i);
}

static int nand_check_status_ll(void)
{
    return NFSTAT & (1<<0);
}

static unsigned char nand_send_data_ll(void) 
{
    return NFDATA;
}

void nand_read_ll(unsigned int addr, unsigned char *buff, unsigned int size)
{
    int index = 0;

    /* The read operation maybe start at any column addr 4 the 1st time */
    unsigned int column = addr % NAND_PAGE_SIZE;
    
    /* Chip select */
    nand_chip_select_ll();

    while (index < size) {
        /* Send command(0x00) */
        nand_send_cmd_ll(0x00);

        /* Send address (5 cycles) */
        nand_send_addr_ll(addr);

        /* Send command(0x30) */
        nand_send_cmd_ll(0x30);

        /* Check the status in order to read data */
        while (!nand_check_status_ll());

        for (; (column < NAND_PAGE_SIZE) && (index < size); column++) {
            /* Read data */
            buff[index] = nand_send_data_ll();
            index++;
            addr++;
        }
        
        /*  
         *  After the 1st read, column is set to zero in order to 
         *  make sure that the next read is correct 
         */
        column = 0;
    }
    
    /* Chip deselect */
    nand_chip_deselect_ll();
}


