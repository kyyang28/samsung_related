
/* Nandflash Controller Registers */
#define NFCONF                  (*(volatile unsigned long *)0x4E000000)
#define NFCONT                  (*(volatile unsigned long *)0x4E000004)
#define NFCMD                   (*(volatile unsigned char *)0x4E000008)
#define NFADDR                  (*(volatile unsigned char *)0x4E00000C)
#define NFDATA                  (*(volatile unsigned char *)0x4E000010)
#define NFSTAT                  (*(volatile unsigned long *)0x4E000020)

#define NAND_PAGE_SIZE          (2048)


/* UART0 registers */
#define ULCON0              (*(volatile unsigned long *)0x50000000)
#define UCON0               (*(volatile unsigned long *)0x50000004)
#define UFCON0              (*(volatile unsigned long *)0x50000008)
#define UMCON0              (*(volatile unsigned long *)0x5000000c)
#define UTRSTAT0            (*(volatile unsigned long *)0x50000010)
#define UTXH0               (*(volatile unsigned char *)0x50000020)
#define URXH0               (*(volatile unsigned char *)0x50000024)
#define UBRDIV0             (*(volatile unsigned long *)0x50000028)

/* GPIO for UART0 */
#define GPHCON              (*(volatile unsigned long *)0x56000070)
#define GPHDAT              (*(volatile unsigned long *)0x56000074)
#define GPHUP               (*(volatile unsigned long *)0x56000078)

#define TXD0READY           (1<<2)
#define PCLK                (50000000)
#define UART_CLK            (PCLK)        
#define UART_BAUD_RATE      (115200)
#define UART_BRD            ((UART_CLK  / (UART_BAUD_RATE * 16)) - 1)


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


void uart0_init(void)
{
    GPHCON  |= 0xa0;    // GPH2,GPH3 is set to TXD0,RXD0
    GPHUP   = 0x0c;     // GPH2,GPH3 pull up

    ULCON0  = 0x03;     // 8 N 1
    UCON0   = 0x05;     // POLL Mode£¬UART clk source is PCLK
    UFCON0  = 0x00;     // No FIFO
    UMCON0  = 0x00;     // No flow control
    UBRDIV0 = UART_BRD; // baudrate is 115200
}

void putc(unsigned char c)
{
    /* Wait untill the buffer data is sent completely */
    while (!(UTRSTAT0 & TXD0READY));
    
    /* Write data to UTXH0, then UART sends the data automatically */
    UTXH0 = c;
}

void puts(char *s)
{
    while (*s != '\0')
        putc(*s++);
}

