
#ifndef __S3C2440_H__
#define __S3C2440_H__


typedef unsigned char                   u8;
typedef unsigned int                    u32;
typedef volatile u8                     S3C2440_REG8;
typedef volatile u32                    S3C2440_REG32;	


/* GPIO */
#define GPECON                          (*(S3C2440_REG32 *)0x56000040)   // IIC
#define GPEDAT                          (*(S3C2440_REG32 *)0x56000044)
#define GPEUP                           (*(S3C2440_REG32 *)0x56000048)


/* LED KEY */
#define GPFCON                          (*(S3C2440_REG32 *)0x56000050)
#define GPFDAT                          (*(S3C2440_REG32 *)0x56000054)
#define GPGCON                          (*(S3C2440_REG32 *)0x56000060)
#define GPGDAT                          (*(S3C2440_REG32 *)0x56000064)


/* CLOCK */
#define	S3C2440_CLOCK_BASE		(0x4c000000)
#define MDIV				    (0x5c)
#define PDIV				    (0x01)
#define SDIV				    (0x01)


typedef struct {
	S3C2440_REG32 LOCKTIME;
	S3C2440_REG32 MPLLCON;
	S3C2440_REG32 UPLLCON;
	S3C2440_REG32 CLKCON;
	S3C2440_REG32 CLKSLOW;
	S3C2440_REG32 CLKDIVN;
	S3C2440_REG32 CAMDIVN;
}/*__attribute__((__packed__)) */S3C2440_CLOCK;


static inline S3C2440_CLOCK * const S3C2440_Get_CLOCK_Base(void)
{
	return (S3C2440_CLOCK * const)S3C2440_CLOCK_BASE;
}


/* Nandflash  */

#define BUSY                   (1)

#define NAND_SECTOR_SIZE       (512)
#define NAND_BLOCK_MASK        (NAND_SECTOR_SIZE - 1)

#define NAND_SECTOR_SIZE_LP    (2048)
#define NAND_BLOCK_MASK_LP     (NAND_SECTOR_SIZE_LP - 1)

#define S3C2440_NAND_BASE      (0x4E000000)

#define TACLS                  (0)
#define TWRPH0                 (3)
#define TWRPH1                 (0)

/* NAND FLASH */
typedef struct {
    S3C2440_REG32   NFCONF;
    S3C2440_REG32   NFCONT;
    S3C2440_REG32   NFCMD;
    S3C2440_REG32   NFADDR;
    S3C2440_REG32   NFDATA;
    S3C2440_REG32   NFMECCD0;
    S3C2440_REG32   NFMECCD1;
    S3C2440_REG32   NFSECCD;
    S3C2440_REG32   NFSTAT;
    S3C2440_REG32   NFESTAT0;
    S3C2440_REG32   NFESTAT1;
    S3C2440_REG32   NFMECC0;
    S3C2440_REG32   NFMECC1;
    S3C2440_REG32   NFSECC;
    S3C2440_REG32   NFSBLK;
    S3C2440_REG32   NFEBLK;
} S3C2440_NAND_REG;


/* Interrupt */
#define SRCPND                          (*(S3C2440_REG32 *)0x4A000000)
#define INTMOD                          (*(S3C2440_REG32 *)0x4A000004)
#define INTMSK                          (*(S3C2440_REG32 *)0x4A000008)
#define PRIORITY                        (*(S3C2440_REG32 *)0x4A00000C)
#define INTPND                          (*(S3C2440_REG32 *)0x4A000010)
#define INTOFFSET                       (*(S3C2440_REG32 *)0x4A000014)
#define SUBSRCPND                       (*(S3C2440_REG32 *)0x4A000018)
#define INTSUBMSK                       (*(S3C2440_REG32 *)0x4A00001C)

/* External Interrupt */
#define EINTMASK                        (*(S3C2440_REG32 *)0x560000A4)
#define EINTPEND                        (*(S3C2440_REG32 *)0x560000A8)


/* GPHCON for UART0,1,2  */
#define GPHCON				(*(S3C2440_REG32 *)0x56000070)
#define GPHDAT				(*(S3C2440_REG32 *)0x56000074)
#define GPHUP				(*(S3C2440_REG32 *)0x56000078)


/* UART Registers */
/* ULCONx */
#define ULCON0				(*(S3C2440_REG32 *)0x50000000)
#define ULCON1				(*(S3C2440_REG32 *)0x50004000)
#define ULCON2				(*(S3C2440_REG32 *)0x50008000)

/* UCONx */
#define UCON0				(*(S3C2440_REG32 *)0x50000004)
#define UCON1				(*(S3C2440_REG32 *)0x50004004)
#define UCON2				(*(S3C2440_REG32 *)0x50008004)

/* UFCONx */
#define UFCON0				(*(S3C2440_REG32 *)0x50000008)
#define UFCON1				(*(S3C2440_REG32 *)0x50004008)
#define UFCON2				(*(S3C2440_REG32 *)0x50008008)

/* UMCONx */
#define UMCON0				(*(S3C2440_REG32 *)0x5000000C)
#define UMCON1				(*(S3C2440_REG32 *)0x5000400C)

/* UTRSTATx */
#define UTRSTAT0			(*(S3C2440_REG32 *)0x50000010)
#define UTRSTAT1			(*(S3C2440_REG32 *)0x50004010)
#define UTRSTAT2			(*(S3C2440_REG32 *)0x50008010)

/* UTXHx Little endian */
#define UTXH0_L				(*(S3C2440_REG8 *)0x50000020)	
#define UTXH1_L				(*(S3C2440_REG8 *)0x50004020)	
#define UTXH2_L				(*(S3C2440_REG8 *)0x50008020)	

/* URXHx Little endian */
#define URXH0_L				(*(S3C2440_REG8 *)0x50000024)	
#define URXH1_L				(*(S3C2440_REG8 *)0x50004024)	
#define URXH2_L				(*(S3C2440_REG8 *)0x50008024)	

/* UBRDIVx */
#define UBRDIV0				(*(S3C2440_REG32 *)0x50000028)
#define UBRDIV1				(*(S3C2440_REG32 *)0x50004028)
#define UBRDIV2				(*(S3C2440_REG32 *)0x50008028)


/* UART */
#define PCLK				(50000000)
#define UART_CLOCK			(PCLK)
#define UART_BAUDRATE                   (115200)

#define TX_READY			(1<<2)
#define RX_READY			(1)


/* IIC */
#define IICCON                          (*(S3C2440_REG32 *)0x54000000)
#define IICSTAT                         (*(S3C2440_REG32 *)0x54000004)
#define IICADD                          (*(S3C2440_REG32 *)0x54000008)
#define IICDS                           (*(S3C2440_REG32 *)0x5400000C)
#define IICLC                           (*(S3C2440_REG32 *)0x54000000)

#define WRITE                           (0)
#define READ                            (1)

/* Interrupt offset  */
#define EINT0_OFFSET			(0) 
#define EINT1_OFFSET			(1) 
#define EINT2_OFFSET			(2) 
#define EINT3_OFFSET			(3) 
#define EINT4_OFFSET			(4) 
#define IIC_OFFSET			(27)

#endif

