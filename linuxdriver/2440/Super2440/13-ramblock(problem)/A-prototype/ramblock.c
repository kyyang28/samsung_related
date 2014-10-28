
/*
 *  Referenced by linux-2.6.38\drivers\block\xd.c and linux-2.6.38\drivers\block\z2ram.c
 */

#include <linux/module.h>
#include <linux/errno.h>
#include <linux/interrupt.h>
#include <linux/mm.h>
#include <linux/fs.h>
#include <linux/kernel.h>
#include <linux/timer.h>
#include <linux/genhd.h>
#include <linux/hdreg.h>
#include <linux/ioport.h>
#include <linux/init.h>
#include <linux/wait.h>
#include <linux/blkdev.h>
#include <linux/mutex.h>
#include <linux/blkpg.h>
#include <linux/delay.h>
#include <linux/io.h>
#include <linux/gfp.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>


#define RAMBLOCK_SIZE   (1024 * 1024)   /* 1M */

static struct gendisk *ramblock_disk;
static struct request_queue *ramblock_queue;

static int major;

static const struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
//	.ioctl	= ramblock_ioctl,
//	.getgeo = ramblock_getgeo,
};

static DEFINE_SPINLOCK(ramblock_lock);


static void do_ramblock_request (struct request_queue * q)
{
    static int cnt = 0;
	struct request *req;

    printk("do_ramblock_request: cnt = %d\n", ++cnt);

	while ((req = blk_fetch_request(q)) != NULL) {
        __blk_end_request_cur(req, 0);
#if 0
        if (!__blk_end_request_cur(req, 0)) {
			//req = blk_fetch_request(q);     
        }
#endif
    }
}

static int __init ramblock_init(void)
{
    /* 1. Allocate gendisk struct */
    ramblock_disk = alloc_disk(16);

    /* 2. Configure */
    /* 2.1 Allocate/Configure a queue which supporting read/write capabilities */
	if (!(ramblock_queue = blk_init_queue(do_ramblock_request, &ramblock_lock)))
		return -ENOMEM;
	ramblock_disk->queue = ramblock_queue;


    /* 2.2 Configure other properties: such as volume, etc */
    major = register_blkdev(0, "ramblock");
    
	ramblock_disk->major = major;
	ramblock_disk->first_minor = 0;
	sprintf(ramblock_disk->disk_name, "ramblock");
	ramblock_disk->fops = &ramblock_fops;
	set_capacity(ramblock_disk, RAMBLOCK_SIZE / 512);  /* 512 bytes per sector */


    /* 3. Register */
    add_disk(ramblock_disk);
    
    return 0;
}


static void __exit ramblock_exit(void)
{
    unregister_blkdev(major, "ramblock");
    del_gendisk(ramblock_disk);
    put_disk(ramblock_disk);
    blk_cleanup_queue(ramblock_queue);
}


module_init(ramblock_init);
module_exit(ramblock_exit);

MODULE_LICENSE("Dual BSD/GPL");


