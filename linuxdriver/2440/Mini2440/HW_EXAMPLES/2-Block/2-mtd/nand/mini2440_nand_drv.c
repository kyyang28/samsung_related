

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
#include <linux/ioport.h>
 
#include <linux/mtd/mtd.h>
#include <linux/mtd/nand.h>
#include <linux/mtd/nand_ecc.h>
#include <linux/mtd/partitions.h>
 
#include <asm/io.h>

#include <plat/regs-nand.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


struct mini2440_nand {
    struct nand_chip *chip;
    struct mtd_info *mtd;
    struct resource *nand_mem;
    void __iomem *nand_io;
};

static struct mini2440_nand nand;

static struct mtd_partition mini2440_nand_parts[] = {
	[0] = {
        .name   = "u-boot",
        .size   = SZ_256K,
		.offset	= 0,
	},
	[1] = {
        .name   = "u-boot-env",
        .offset = MTDPART_OFS_APPEND,
        .size   = SZ_128K,
	},
	[2] = {
        .name   = "kernel",
        .offset = MTDPART_OFS_APPEND,
        .size   = SZ_4M,
	},
	[3] = {
        .name   = "root",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,
	}
};


static void mini2440_select_chip(struct mtd_info *mtd, int chipnr)
{
	if (chipnr == -1)
		/* ȡ��ѡ��: NFCONT[1]��Ϊ1 */
        writel((readl(nand.nand_io + S3C2440_NFCONT) | (1<<1)), nand.nand_io + S3C2440_NFCONT);
	else
		/* ѡ��: NFCONT[1]��Ϊ0 */
        writel((readl(nand.nand_io + S3C2440_NFCONT) & ~(1<<1)), nand.nand_io + S3C2440_NFCONT);
}

static void mini2440_cmd_ctrl(struct mtd_info *mtd, int dat, unsigned int ctrl)
{
	if (ctrl & NAND_CLE)
		/* ������: NFCMMD=dat */
        writel(dat, nand.nand_io + S3C2440_NFCMD);
	else if (ctrl & NAND_ALE)
		/* ����ַ: NFADDR=dat */
        writel(dat, nand.nand_io + S3C2440_NFADDR);
}

static int mini2440_dev_ready(struct mtd_info *mtd)
{
	return (readl(nand.nand_io + S3C2440_NFSTAT) & (1<<0));
}


static int mini2440_nand_drv_probe(struct platform_device *pdev)
{
    struct clk *clk;
    struct resource *res;
    int retval;
    int size;
    unsigned long tmp;
    
    /* 1. ����һ��nand_chip�ṹ�� */
    nand.chip = kzalloc(sizeof(struct nand_chip), GFP_KERNEL);
    
    res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (res == NULL) {
        dev_err(&pdev->dev, "failed to get memory registers\n");
        retval = -ENXIO;
        goto dealloc_nand_chip;
    }

	size = resource_size(res);
	nand.nand_mem = request_mem_region(res->start, size, pdev->name);
	if (nand.nand_mem == NULL) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		retval = -ENOENT;
		goto dealloc_nand_chip;
	}

	nand.nand_io = ioremap(res->start, size);
	if (nand.nand_io == NULL) {
		dev_err(&pdev->dev, "ioremap() of registers failed\n");
		retval = -ENXIO;
		goto release_mem;
	}
        
    /* 2. ����nand_chip */
    /* ����nand_chip�Ǹ�nand_scan����ʹ�õ�, �����֪����ô����, �ȿ�nand_scan��ôʹ�� 
     * ��Ӧ���ṩ:ѡ��,������,����ַ,������,������,�ж�״̬�Ĺ���
     */
    nand.chip->select_chip = mini2440_select_chip;
    nand.chip->cmd_ctrl    = mini2440_cmd_ctrl;
    nand.chip->IO_ADDR_R   = nand.nand_io + S3C2440_NFDATA;
    nand.chip->IO_ADDR_W   = nand.nand_io + S3C2440_NFDATA;
    nand.chip->dev_ready   = mini2440_dev_ready;
    nand.chip->ecc.mode    = NAND_ECC_SOFT;
    
    /* 3. Ӳ����ص�����: ����NAND FLASH���ֲ�����ʱ����� */
    /* ʹ��NAND FLASH��������ʱ�� */
    clk = clk_get(NULL, "nand");
    clk_enable(clk);              /* CLKCON'bit[4] */
    
    /* HCLK=100MHz
     * TACLS:  ����CLE/ALE֮��೤ʱ��ŷ���nWE�ź�, ��NAND�ֲ��֪CLE/ALE��nWE����ͬʱ����,����TACLS=0
     * TWRPH0: nWE��������, HCLK x ( TWRPH0 + 1 ), ��NAND�ֲ��֪��Ҫ>=12ns, ����TWRPH0>=1
     * TWRPH1: nWE��Ϊ�ߵ�ƽ��೤ʱ��CLE/ALE���ܱ�Ϊ�͵�ƽ, ��NAND�ֲ��֪��Ҫ>=5ns, ����TWRPH1>=0
     */
#define TACLS    0
#define TWRPH0   1
#define TWRPH1   0
    tmp = (TACLS<<12) | (TWRPH0<<8) | (TWRPH1<<4);
    writel(tmp, nand.nand_io + S3C2410_NFCONF);

    /* NFCONT: 
     * BIT1-��Ϊ1, ȡ��Ƭѡ 
     * BIT0-��Ϊ1, ʹ��NAND FLASH������
     */
    tmp = (1<<1) | (1<<0);
    writel(tmp, nand.nand_io + S3C2440_NFCONT);
    
    /* 4. ʹ��: nand_scan */
    nand.mtd = kzalloc(sizeof(struct mtd_info), GFP_KERNEL);
    nand.mtd->owner = THIS_MODULE;
    nand.mtd->priv  = nand.chip;
    
	/* Scan to find existence of the device */
	if (nand_scan(nand.mtd, 1)) {
		retval = -ENODEV;
		goto err_scan;
	}
    
    /* 5. add_mtd_partitions */
    mtd_device_register(nand.mtd, mini2440_nand_parts, 4);
        
    return 0;

release_mem:
    release_mem_region(res->start, size);        
dealloc_nand_chip:
    kfree(nand.chip);
err_scan:
    kfree(nand.mtd);    
    return retval;
}


static int mini2440_nand_drv_remove(struct platform_device *pdev)
{
	mtd_device_unregister(nand.mtd);
	kfree(nand.mtd);
	iounmap(nand.nand_io);
	kfree(nand.chip);
    return 0;
}


static struct platform_driver mini2440_driver_nand = {
    .probe              = mini2440_nand_drv_probe,
    .remove             = __devexit_p(mini2440_nand_drv_remove),
    .driver             = {
        .owner      = THIS_MODULE,
        .name       = "mini2440-nand",
    },
};

static int __init mini2440_nand_drv_init(void)
{
    return platform_driver_register(&mini2440_driver_nand);
}

static void __exit mini2440_nand_drv_exit(void)
{
    platform_driver_unregister(&mini2440_driver_nand);
}

module_init(mini2440_nand_drv_init);
module_exit(mini2440_nand_drv_exit);

