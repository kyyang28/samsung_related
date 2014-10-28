

/*  
 * Referenced by linux-2.6.38\drivers\mtd\maps\physmap.c
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
#include <linux/mtd/concat.h>
#include <linux/io.h>


static struct map_info *s3c_nor_map;
static struct mtd_info *s3c_nor_mtd_info;

static int __init s3c_nor_init(void)
{
    /* 1. Allocate map_info struct */
    s3c_nor_map = kzalloc(sizeof(struct map_info), GFP_KERNEL);

    /* 2. Configure map_info: phys base addr, size, bankwidth, virtual base addr */
	s3c_nor_map->name = "s3c_nor";
	s3c_nor_map->phys = 0x0;
	s3c_nor_map->size = 0x1000000;  /* >= Nor's really size */
	s3c_nor_map->bankwidth = 2;
	s3c_nor_map->virt = ioremap(s3c_nor_map->phys, s3c_nor_map->size);

    simple_map_init(s3c_nor_map);
    
    /* 3. Use(Apply): Invoking the functions from the Norflash protocol layer to identify */
    printk("Use cfi_probe!\n");
    s3c_nor_mtd_info = do_map_probe("cfi_probe", s3c_nor_map);
    if (!s3c_nor_mtd_info) {
        printk("Use jedec_probe!\n");
        s3c_nor_mtd_info = do_map_probe("jedec_probe", s3c_nor_map);    
    }

    if (!s3c_nor_mtd_info) {
        iounmap(s3c_nor_map->virt);
        kfree(s3c_nor_map);
        return -EIO;
    }
    
    /* 4. add_mtd_partitions */
    
    
    return 0;
}


static void __exit s3c_nor_exit(void)
{
    iounmap(s3c_nor_map->virt);
    kfree(s3c_nor_map);
}

module_init(s3c_nor_init);
module_exit(s3c_nor_exit);

MODULE_LICENSE("Dual BSD/GPL");


