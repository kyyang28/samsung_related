
/*  
 * Referenced by linux-2.6.38\drivers\mtd\nand\s3c2410.c and
 *               linux-2.6.38\drivers\mtd\nand\atmel_nand.c
 */


#include <linux/module.h>
#include <linux/types.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/string.h>
#include <linux/ioport.h>
#include <linux/platform_device.h>
#include <linux/delay.h>
#include <linux/err.h>
#include <linux/slab.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>

#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>

#include <asm/io.h>

#include <plat/regs-nand.h>
#include <plat/nand.h>


struct s3c2440_nand_regs {
    unsigned long NFCONF;
    unsigned long NFCONT;
    unsigned long NFCMD;
    unsigned long NFADDR;
    unsigned long NFDATA;
    unsigned long NFECCD0;
    unsigned long NFECCD1;
    unsigned long NFECCD;
    unsigned long NFSTAT;
    unsigned long NFESTAT0;
    unsigned long NFESTAT1;
    unsigned long NFMECC0;
    unsigned long NFMECC1;
    unsigned long NFSECC;
    unsigned long NFSBLK;
    unsigned long NFEBLK;
};


static struct nand_chip *s3c_nand_chip;
static struct mtd_info *s3c_mtd_info;

static struct s3c2440_nand_regs *s3c_nand_regs;


static struct mtd_partition s3c_nand_part[] = {
	[0] = {
		.name	= "Bootloader",
		.offset	= 0,
		.size	= 0x00060000,//0x00060000=384KB
	},
	[1] = {
		.name	= "BootParam",
		.offset = 0x00060000,
		.size	= 0x00020000,//0x00020000=128KB
	},
    [2] = {
        .name   = "BootLogo",
        .offset = 0x00080000,
        .size   = 0x00180000,//0x00180000=1.5MB
        },
	[3] = {
		.name	= "Kernel",
		.offset = 0x00200000,
		.size	= 0x00360000,//0x00360000=3.375MB
	},
	[4] = {
		.name	= "Qt",
		.offset	= 0x00560000,
		.size	= 0x2800000,//0x2800000=40MB
	},
	[5] = {
		.name	= "Rootfs",
		.offset	= 0x02D60000,
		.size	= 1024 * 1024 * 1024,
	},
};


static void s3c2440_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
    if (chipnr == -1) {
        /* Cancel nand select chip */
        s3c_nand_regs->NFCONT |= 1<<1;
    }else {
        /* nand select chip */
        /* NFCONT [1]   0: Force nFCE to low (Enable chip select) */        
        s3c_nand_regs->NFCONT &= ~(1<<1);
    }
}


static void s3c2440_nand_cmd_ctrl(struct mtd_info *mtd, int data, unsigned int ctrl)
{
	if (ctrl & NAND_CLE) {
		/* Send command: NFCMMD = data */
        s3c_nand_regs->NFCMD = data;
	}else {
		/* Send address: NFADDR = data */
        s3c_nand_regs->NFADDR = data;
	}
}


static int s3c2440_nand_device_ready(struct mtd_info *mtd)
{
    return (s3c_nand_regs->NFSTAT & (1<<0));
}


static int __init s3c_nand_init(void)
{
    struct clk *clk;
        
    /*  1. Allocate nand_chip struct */
    s3c_nand_chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);

    s3c_nand_regs = ioremap(0x4E000000, sizeof(struct s3c2440_nand_regs));

    
    /*  2. Configure nand_chip struct */
    s3c_nand_chip->select_chip          = s3c2440_nand_select_chip;
    s3c_nand_chip->cmd_ctrl             = s3c2440_nand_cmd_ctrl;
    s3c_nand_chip->IO_ADDR_R            = &s3c_nand_regs->NFDATA;   /* The virtual address of NFDATA */
    s3c_nand_chip->IO_ADDR_W            = &s3c_nand_regs->NFDATA;   /* The virtual address of NFDATA */
    s3c_nand_chip->dev_ready            = s3c2440_nand_device_ready;
	s3c_nand_chip->ecc.mode             = NAND_ECC_SOFT;	        /* enable ECC */


    /*  3. Hardware related operations */
    /* Enable Nandflash clock */
    clk = clk_get(NULL, "nand");
	if (IS_ERR(clk)) {
		printk("failed to get clock\n");
        iounmap(s3c_nand_regs);
        kfree(s3c_nand_chip);
		return -ENOENT;
	}
    clk_enable(clk);


    /*  According to the nand chip manual, setup the timing parameters 
     *  HCLK = 101.250MHz = 9.88ns
     *  TACLS means after sending the CLE/ALE, how long do we send the nWE signal, TACLS = 0
     *  TWRPH0 means nWE pulse duration, 
     *      Duration = HCLK x ( TWRPH0 + 1 ) >= 12ns ( According to nand chip manual(K9F4G08 p. 11) )
     *      ==> 9.88ns x ( TWRPH0 + 1 ) >= 12ns
     *      ==> TWRPH0 >= 1
     *  TWRPH1 means after the nWE becomes HIGH LEVEL, how long will CLE/ALE becomes LOW LEVEL from HIGH LEVEL.
     *      Duration = HCLK x ( TWRPH1 + 1 ) >= 5ns ( According to nand chip manual(K9F4G08 p. 11) )
     *      ==> 9.88ns x ( TWRPH1 + 1 ) >= 5ns
     *      ==> TWRPH1 >= 0
     */
#define TACLS           0
#define TWRPH0          1
#define TWRPH1          0
    s3c_nand_regs->NFCONF = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
    

    /*  NFCONT [1] - 1: Force nFCE to High (Disable chip select) 
     *               0: Force nFCE to low (Enable chip select)
     *  NFCONT [0] - 1: NAND Flash Controller Enable
     *               0: NAND Flash Controller Disable (Don¡¯t work)
     */
    s3c_nand_regs->NFCONT = (1<<1) | (1<<0);
    

    /*  4. Use: nand_scan_ident */
    s3c_mtd_info = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	s3c_mtd_info->owner = THIS_MODULE;
	s3c_mtd_info->priv = s3c_nand_chip;

    nand_scan(s3c_mtd_info, 1);


    /* Extra: use add_mtd_device when the whole nandflash is refresented by 1 partition */
    //add_mtd_device(s3c_mtd_info);
    
    /*  5. add_mtd_partitions */
    add_mtd_partitions(s3c_mtd_info, s3c_nand_part, 6);


    return 0;
}


static void __exit s3c_nand_exit(void)
{
    del_mtd_partitions(s3c_mtd_info);
    kfree(s3c_mtd_info);
    //clk_disable(clk);
    iounmap(s3c_nand_regs);
    kfree(s3c_nand_chip);
}


module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("Dual BSD/GPL");


