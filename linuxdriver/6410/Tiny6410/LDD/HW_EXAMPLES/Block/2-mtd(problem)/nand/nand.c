#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>
#include <mach/hardware.h>
#include <asm/sizes.h>
#include <asm/mach-types.h>
#include <linux/clk.h>

struct mtd_partition s3c_parts[] = {
        {
                .name		= "Bootloader",
                .offset		= 0,
                .size		= (256*SZ_1K),
        },
        {
                .name		= "Kernel",
                .offset		= MTDPART_OFS_APPEND,
                .size		= (2*SZ_1M),
        },
        {
                .name		= "File System",
                .offset		= MTDPART_OFS_APPEND,
                .size		= MTDPART_SIZ_FULL,
        }
};

struct s3c6410_nand_regs {
	unsigned long  nfconf	;
	unsigned long  nfcont;
	unsigned long  nfcmd;
	unsigned long  nfaddr;
	unsigned long  nfdata;
	unsigned long  nfmeccdata0;
	unsigned long  nfmeccdata1;
	unsigned long  nfseccdata;
	unsigned long  nfsblk;
	unsigned long  nfeblk;
	unsigned long  nfstat;
	unsigned long  nfmeccerr0;
	unsigned long  nfmeccerr1;
	unsigned long  nfmecc0;
	unsigned long  nfmecc1;
	unsigned long  nfsecc;
	unsigned long  nfmlcbitpt;
};


/*
 * drivers\mtd\nand\atmel_nand.c
 */

static struct nand_chip *s3c_nand;
static struct mtd_info *s3c_mtd;
static struct s3c6410_nand_regs *s3c_nand_regs;
	
static void s3c6410_nand_select(struct mtd_info *mtd, int chip)
{
	if (chip == -1)
	{
		/* 取消片选 */
		s3c_nand_regs->nfcont |= (1<<1);
	}
	else
	{
		/* 选中 */
		s3c_nand_regs->nfcont &= ~(1<<1);
	}
}

/*
 * Hardware specific access to control-lines
 */
static void s3c6410_nand_cmd_addr(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
	{
		/* 发命令 */
		s3c_nand_regs->nfcmd = dat;
	}
	else
	{
		/* 发地址 */
		s3c_nand_regs->nfaddr = dat;
	}
}

static int s3c6410_nand_ready(struct mtd_info *mtd)
{
	return (s3c_nand_regs->nfstat & (1<<0));
}

static int s3c6410_nand_init(void)
{
	struct clk *clk;
	
	s3c_nand_regs = ioremap(0x70200000, sizeof(struct s3c6410_nand_regs));
	
	/* 1. 分配nand_chip */
	s3c_nand = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
	s3c_mtd  = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	s3c_mtd->priv = s3c_nand;
	s3c_mtd->owner = THIS_MODULE;
    
    
	/* 2. 设置s3c_nand */
	s3c_nand->select_chip   = s3c6410_nand_select;
	s3c_nand->cmd_ctrl      = s3c6410_nand_cmd_addr;
	s3c_nand->IO_ADDR_R     = &s3c_nand_regs->nfdata;
	s3c_nand->IO_ADDR_W     = &s3c_nand_regs->nfdata;
	s3c_nand->dev_ready     = s3c6410_nand_ready;
	s3c_nand->ecc.mode      = NAND_ECC_SOFT;	/* enable ECC */
	
	/* 3. 硬件相关的操作 */
	clk = clk_get(NULL, "nand");
	clk_enable(clk);

	/* 时序设置 :
	 * 看6410手册确定能设置哪些参数
	 * 再看NAND手册确定这些参数设为什么值
	 */
#define TACLS    1
#define TWRPH0   2
#define TWRPH1   1
	s3c_nand_regs->nfconf = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4) | (1<<2);
	s3c_nand_regs->nfcont = 0x07;
	
	/* 4. 使用s3c_nand */
	nand_scan(s3c_mtd, 1);
	/* 识别成功的话, s3c_mtd->read,write,erase就被设为nand_read,nand_write,nand_erase */
	
	add_mtd_partitions(s3c_mtd, s3c_parts, 3);
	
	return 0;
}

static void s3c6410_nand_exit(void)
{
	del_mtd_partitions(s3c_mtd);
	kfree(s3c_mtd);
	kfree(s3c_nand);
	iounmap(s3c_nand_regs);
}

module_init(s3c6410_nand_init);
module_exit(s3c6410_nand_exit);

MODULE_LICENSE("GPL");

