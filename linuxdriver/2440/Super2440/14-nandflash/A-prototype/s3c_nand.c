
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

static struct nand_chip *s3c_nand_chip;
static struct mtd_info *s3c_mtd_info;


static void s3c2440_nand_select_chip(struct mtd_info *mtd, int chipnr)
{
    if (chipnr == -1) {
        /* Cancel nand select chip */
        
    }else {
        /* nand select chip */
        /* NFCONT [1]   0: Force nFCE to low (Enable chip select) */
    }
}


static void s3c2440_nand_cmd_ctrl(struct mtd_info *mtd, int data, unsigned int ctrl)
{
	if (ctrl & NAND_CLE) {
		/* Send command: NFCMMD = data */
	}else {
		/* Send address: NFADDR = data */
	}
}


static int s3c2440_nand_device_ready(struct mtd_info *mtd)
{
    return /* NFSTAT [0] */
}


static int __init s3c_nand_init(void)
{
    /* 1. Allocate nand_chip struct */
    s3c_nand_chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
    
    /* 2. Configure nand_chip struct */
    s3c_nand_chip->select_chip          = s3c2440_nand_select_chip;
    s3c_nand_chip->cmd_ctrl             = s3c2440_nand_cmd_ctrl;
    s3c_nand_chip->IO_ADDR_R            = ; /* The virtual address of NFDATA */
    s3c_nand_chip->IO_ADDR_W            = ; /* The virtual address of NFDATA */
    s3c_nand_chip->dev_ready            = s3c2440_nand_device_ready;

    /* 3. Hardware related operations */
    
    
    /* 4. Use: nand_scan_ident */
    s3c_mtd_info = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
	s3c_mtd_info->owner = THIS_MODULE;
	s3c_mtd_info->priv = s3c_nand_chip;

    nand_scan(s3c_mtd_info, 1);
    
    /* 5. add_mtd_partitions */
    

    return 0;
}


static void __exit s3c_nand_exit(void)
{
}


module_init(s3c_nand_init);
module_exit(s3c_nand_exit);

MODULE_LICENSE("Dual BSD/GPL");


