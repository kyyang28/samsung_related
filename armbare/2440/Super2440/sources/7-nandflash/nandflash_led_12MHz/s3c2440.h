
#ifndef __S3C2440_H__
#define __S3C2440_H__

typedef unsigned int	u32;
typedef volatile u32	S3C2440_REG32;


/* LED */
#define GPFCON                          (*(volatile unsigned long *)0x56000050)
#define GPFDAT                          (*(volatile unsigned long *)0x56000054)

#define GPGCON                          (*(volatile unsigned int *)0x56000060)
#define GPGDAT                          (*(volatile unsigned int *)0x56000064)


/* Nandflash  */
#define LARGER_NAND_PAGE

#define BUSY            	1

#define NAND_SECTOR_SIZE    	512
#define NAND_BLOCK_MASK     (NAND_SECTOR_SIZE - 1)

#define NAND_SECTOR_SIZE_LP    	2048
#define NAND_BLOCK_MASK_LP     (NAND_SECTOR_SIZE_LP - 1)

#define S3C2440_NAND_BASE      (0x4E000000)

#define TACLS                (0)
#define TWRPH0               (3)
#define TWRPH1               (0)

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


typedef struct {
    void (*nand_reset)(void);
    void (*wait_idle)(void);
    void (*nand_select_chip)(void);
    void (*nand_deselect_chip)(void);
    void (*write_cmd)(int cmd);
    void (*write_addr)(unsigned int addr);
    unsigned char (*read_data)(void);
} S3C2440_NAND_CMD;


void nand_init(void);
void nand_read(unsigned char *buf, unsigned long start_addr, int size);

void nand_reset(void);
void wait_idle(void);
void nand_select_chip(void);
void nand_deselect_chip(void);
void write_cmd(int cmd);
void write_addr(unsigned int addr);
unsigned char read_data(void);


void s3c2440_nand_reset(void);
void s3c2440_wait_idle(void);
void s3c2440_nand_select_chip(void);
void s3c2440_nand_deselect_chip(void);
void s3c2440_write_cmd(int cmd);
//void s3c2440_write_addr(unsigned int addr);
unsigned char s3c2440_read_data(void);


#endif

