#include <linux/slab.h>
#include <linux/init.h>
#include <linux/module.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <mach/hardware.h>	/* for CLPS7111_VIRT_BASE */
#include <asm/sizes.h>
#include <linux/clk.h>

/* drivers\mtd\nand\h1910.c */

static struct mtd_info *s3c6410_mtd;
static struct nand_chip *s3c_nand_chip;

struct s3c6410_nand_regs {
	unsigned long  NFCONF	;
	unsigned long  NFCONT;
	unsigned long  NFCMD;
	unsigned long  NFADDR;
	unsigned long  NFDATA;
	unsigned long  NFMECCDATA0;
	unsigned long  NFMECCDATA1;
	unsigned long  NFSECCDATA;
	unsigned long  NFSBLK;
	unsigned long  NFEBLK;
	unsigned long  NFSTAT;
	unsigned long  NFMECCERR0;
	unsigned long  NFMECCERR1;
	unsigned long  NFMECC0;
	unsigned long  NFMECC1;
	unsigned long  NFSECC;
	unsigned long  NFMLCBITPT;
};

static struct s3c6410_nand_regs *s3c6410_nand_regs;

static struct mtd_partition s3c_partitions[] = {
        {
                .name		= "Bootloader",
                .offset		= 0,
                .size		= (256*SZ_1K),
                .mask_flags	= MTD_CAP_NANDFLASH,
        },
        {
                .name		= "Kernel",
                .offset		= (256*SZ_1K),
                .size		= (4*SZ_1M) - (256*SZ_1K),
                .mask_flags	= MTD_CAP_NANDFLASH,
        },
        {
                .name		= "Rootfs",
                .offset		= (4*SZ_1M),
                .size		= (80*SZ_1M),//(48*SZ_1M),
        },
        {
                .name		= "File System",
                .offset		= MTDPART_OFS_APPEND,
                .size		= MTDPART_SIZ_FULL,
        }
};

static void s3c6410_nand_select(struct mtd_info *mtd, int chip)
{
	if (0 == chip)
	{
		/* select chip */
		s3c6410_nand_regs->NFCONT &= ~(1<<1);
		//printk("s3c6410_nand_select : %x\n", s3c6410_nand_regs->NFCONT);
	}
	else
	{
		/* deselect chip */
		//printk("s3c6410_nand_deselect\n");
		s3c6410_nand_regs->NFCONT |= (1<<1);
	}
}

static void s3c6410_nand_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	//printk("ctrl = %x\n", ctrl);
	if (ctrl & NAND_CLE)
	{
		//printk("send cmd: %x\n", dat);
		s3c6410_nand_regs->NFCMD = dat;
	}
	else if (ctrl & NAND_ALE)
	{
		//printk("send addr: %x\n", dat);
		s3c6410_nand_regs->NFADDR = dat;
	}
}

static void s3c6410_nand_hw_init(void)
{
//	MEM_SYS_CFG &= ~(1<<1);

	/* setup time parameter */
#define TACLS     7
#define TWRPH0    7
#define TWRPH1    7
	s3c6410_nand_regs->NFCONF &= ~((1<<30) | (7<<12) | (7<<8) | (7<<4));
	s3c6410_nand_regs->NFCONF |= ((TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4));

	/* enable nand flash controller */
	s3c6410_nand_regs->NFCONT |= 1;
	s3c6410_nand_regs->NFCONT &= ~(1<<16); /* disable soft lock */

}

static int s3c6410_nand_ready(struct mtd_info *mtd)
{
	return (s3c6410_nand_regs->NFSTAT & 0x1);
}

static int s3c6410_nand_init(void)
{
	//struct nand_chip *chip;
	struct clk *clk;
    volatile unsigned long *MEM_SYS_CFG;
    
    s3c_nand_chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

	MEM_SYS_CFG = ioremap(0x7E00F120, 4);
	*MEM_SYS_CFG &= ~(1<<1);
	iounmap(MEM_SYS_CFG);

	clk = clk_get(NULL, "nand");
	clk_enable(clk);

	s3c6410_nand_regs = ioremap(0x70200000, 1024);
	s3c6410_nand_hw_init();
	
	//s3c6410_mtd = kzalloc(sizeof(struct mtd_info) + sizeof(struct nand_chip), GFP_KERNEL);
    s3c6410_mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	//chip = &s3c6410_mtd[1];

	s3c6410_mtd->priv = s3c_nand_chip;
	s3c6410_mtd->owner = THIS_MODULE;

	/* setup nand_chip: */
	s3c_nand_chip->options      = 0;
	s3c_nand_chip->select_chip  = s3c6410_nand_select;
	s3c_nand_chip->cmd_ctrl     = s3c6410_nand_cmd_ctrl;
	s3c_nand_chip->IO_ADDR_R    = &s3c6410_nand_regs->NFDATA;
	s3c_nand_chip->IO_ADDR_W    = &s3c6410_nand_regs->NFDATA;
	s3c_nand_chip->dev_ready    = s3c6410_nand_ready;
	s3c_nand_chip->ecc.mode     = NAND_ECC_SOFT;
	
	if (nand_scan(s3c6410_mtd, 1))
	{
		printk("can't find nand flash for s3c6410\n");
		kfree(s3c6410_mtd);
		iounmap(s3c6410_nand_regs);
        kfree(s3c_nand_chip);
		return -EAGAIN;
	}

	add_mtd_partitions(s3c6410_mtd, s3c_partitions, 4);  /* add_disk */
	
	return 0;
}

static void s3c610_nand_exit(void)
{
	del_mtd_partitions(s3c6410_mtd);
	kfree(s3c6410_mtd);
	iounmap(s3c6410_nand_regs);
    kfree(s3c_nand_chip);
}

module_init(s3c6410_nand_init);
module_exit(s3c610_nand_exit);

MODULE_LICENSE("GPL");



