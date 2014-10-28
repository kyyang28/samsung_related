
/*
 * Referenced by drivers\mtd\maps\physmap.c
 */

#include <linux/module.h>
#include <linux/types.h>
#include <linux/kernel.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/device.h>
#include <linux/platform_device.h>
#include <linux/mtd/mtd.h>
#include <linux/mtd/map.h>
#include <linux/mtd/partitions.h>
#include <asm/io.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


static struct map_info *mini2440_nor;
static struct mtd_info *mini2440_nor_mtd;

static struct mtd_partition mini2440_nor_parts[] = {
	[0] = {
        .name   = "u-boot_nor",
        .size   = 0x00040000,
		.offset	= 0,
	},
	[1] = {
        .name   = "root_nor",
        .offset = MTDPART_OFS_APPEND,
        .size   = MTDPART_SIZ_FULL,
	}
};

static int mini2440_nor_init(void)
{
	/* 1. 分配map_info结构体 */
	mini2440_nor = kzalloc(sizeof(struct map_info), GFP_KERNEL);;
	
	/* 2. 设置: 物理基地址(phys), 大小(size), 位宽(bankwidth), 虚拟基地址(virt) */
	mini2440_nor->name = "mini2440_nor";
	mini2440_nor->phys = 0;
	mini2440_nor->size = 0x1000000; /* >= NOR的真正大小 */
	mini2440_nor->bankwidth = 2;
	mini2440_nor->virt = ioremap(mini2440_nor->phys, mini2440_nor->size);

	simple_map_init(mini2440_nor);
	
	/* 3. 使用: 调用NOR FLASH协议层提供的函数来识别 */
	printk("use cfi_probe\n");
	mini2440_nor_mtd = do_map_probe("cfi_probe", mini2440_nor);
	if (!mini2440_nor_mtd)
	{
		printk("use jedec_probe\n");
		mini2440_nor_mtd = do_map_probe("jedec_probe", mini2440_nor);
	}

	if (!mini2440_nor_mtd)
	{		
		iounmap(mini2440_nor->virt);
		kfree(mini2440_nor);
		return -EIO;
	}
	
	/* 4. add_mtd_partitions */
	mtd_device_register(mini2440_nor_mtd, mini2440_nor_parts, 2);
	
	return 0;
}

static void mini2440_nor_exit(void)
{
	mtd_device_unregister(mini2440_nor_mtd);
	iounmap(mini2440_nor->virt);
	kfree(mini2440_nor);
}

module_init(mini2440_nor_init);
module_exit(mini2440_nor_exit);

