
/* Referenced by drivers\block\xd.c drivers\block\z2ram.c */


#include <linux/module.h>               
#include <linux/kernel.h>               /* printk */
#include <linux/errno.h>                /* error numbers */
#include <linux/types.h>                
#include <linux/spinlock_types.h>       /* spinlock_t */
#include <linux/blkdev.h>               /* request_queue structure */
#include <linux/genhd.h>                /* gendisk structure */
#include <linux/timer.h>                /* timer_list structure */
#include <linux/slab.h>                 /* kmalloc */
#include <linux/vmalloc.h>              /* vmalloc */
#include <linux/blk_types.h>            /* bio_vec structure */
#include <linux/hdreg.h>                /* hd_geometry structure */
#include <linux/fs.h>                   /* block_device structure */


#define MINI2440_RAMDISK_MINOR               (16)
#define MINI2440_RAMDISK_SIZE                (1024*1024)
#define MINI2440_RAMDISK_SECTOR_SIZE         (512)

static int mini2440_ramdisk_major = 0;

struct mini2440_ramdisk_dev {
    struct gendisk *gdisk;
    struct request_queue *queue;
    spinlock_t lock;
    unsigned char *ramdisk_buffer;
};

/* Instance of the ramdisk_dev pointer */
static struct mini2440_ramdisk_dev *mini2440_ramdisk_devp;

static int mini2440_ramdisk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    /* Capacity = heads * cylinders * sectors * RAMDISK_SECTOR_SIZE(512) */
    geo->heads      = 2;
    geo->cylinders  = 32;
    geo->sectors    = MINI2440_RAMDISK_SIZE/geo->heads/geo->cylinders/MINI2440_RAMDISK_SECTOR_SIZE;

    return 0;
}

static struct block_device_operations mini2440_ramdisk_fops = {
    .owner      = THIS_MODULE,
    .getgeo     = mini2440_ramdisk_getgeo,       /* for fdisk */
};

static void mini2440_ramdisk_request(struct request_queue *q)
{    
	static int r_cnt = 0;
	static int w_cnt = 0;
	struct request *req;
	
	req = blk_fetch_request(q);
	while (req) {
		/* 数据传输三要素: 源,目的,长度 */
		/* 源/目的: */
		unsigned long offset = blk_rq_pos(req) << 9;

		/* 长度: */		
		unsigned long len = blk_rq_cur_bytes(req);

		if (rq_data_dir(req) == READ) {
			printk("[RAMDISK]ramdisk_request read %d\n", ++r_cnt);
			memcpy(req->buffer, mini2440_ramdisk_devp->ramdisk_buffer+offset, len);
		}else {
			printk("[RAMDISK]ramdisk_request write %d\n", ++w_cnt);
			memcpy(mini2440_ramdisk_devp->ramdisk_buffer+offset, req->buffer, len);
		}		
		
		if (!__blk_end_request_cur(req, 0))
			req = blk_fetch_request(q);
		else
			printk("[RAMDISK]__blk_end_request_cur error!\n");
	}
}


static int __init mini2440_ramdisk_init(void)
{
    int ret = 0;
    
    /* Register block device */
    mini2440_ramdisk_major = register_blkdev(mini2440_ramdisk_major, "mini2440_ramdisk"); /* cat /proc/devices */
    
    /* Allocating ramdisk_dev structure */
    mini2440_ramdisk_devp = kmalloc(sizeof(struct mini2440_ramdisk_dev), GFP_KERNEL);
    if (!mini2440_ramdisk_devp) {
        printk(KERN_NOTICE "[RAMDISK]allocating ramdisk_devp is failed!\n");
        ret = -ENOMEM;
        goto error_ramdisk_devp;
    }
    
    /* Allocating a gendisk structure */
    mini2440_ramdisk_devp->gdisk = alloc_disk(MINI2440_RAMDISK_MINOR);
    if (!mini2440_ramdisk_devp->gdisk) {
        printk(KERN_NOTICE "[RAMDISK]alloc_disk is failed!\n");
        ret = -ENOMEM;
        goto error_request_queue_gendisk;
    }

    /* Initialize spinlock_t structure */
    spin_lock_init(&mini2440_ramdisk_devp->lock);

    /* 
     * Allocating request_queue structure and config it in order to 
     * support read/write capabilities 
     */
    mini2440_ramdisk_devp->queue = blk_init_queue(mini2440_ramdisk_request, &mini2440_ramdisk_devp->lock);
    if (!mini2440_ramdisk_devp->queue) {
        printk(KERN_NOTICE "[RAMDISK]blk_init_queue is failed!\n");
        ret = -ENOMEM;
        goto error_request_queue_gendisk;
    }

    /* Store the ramdisk_devp to queuedata */
    mini2440_ramdisk_devp->queue->queuedata = mini2440_ramdisk_devp;
    
    /* Config other characteristics, such as capacity */
    mini2440_ramdisk_devp->gdisk->queue          = mini2440_ramdisk_devp->queue;
    mini2440_ramdisk_devp->gdisk->major          = mini2440_ramdisk_major;
    mini2440_ramdisk_devp->gdisk->first_minor    = 0;
    mini2440_ramdisk_devp->gdisk->fops           = &mini2440_ramdisk_fops;
    mini2440_ramdisk_devp->gdisk->private_data   = mini2440_ramdisk_devp;       
    sprintf(mini2440_ramdisk_devp->gdisk->disk_name, "mini2440_ramdisk");
    set_capacity(mini2440_ramdisk_devp->gdisk, MINI2440_RAMDISK_SIZE / MINI2440_RAMDISK_SECTOR_SIZE);

    mini2440_ramdisk_devp->ramdisk_buffer = kzalloc(MINI2440_RAMDISK_SIZE, GFP_KERNEL);
    if (!mini2440_ramdisk_devp->ramdisk_buffer) {
        printk(KERN_NOTICE "[RAMDISK]allocating ramdisk_buffer is failed!\n");
        ret = -ENOMEM;
        goto error_ramdisk_buffer;
    }

    /* Adding(Registering) gendisk */
    add_disk(mini2440_ramdisk_devp->gdisk);

    return ret;

error_ramdisk_devp:
    unregister_blkdev(mini2440_ramdisk_major, "ramdisk");
    return ret;
    
error_request_queue_gendisk:
    kfree(mini2440_ramdisk_devp);
    unregister_blkdev(mini2440_ramdisk_major, "ramdisk");
    return ret;

error_ramdisk_buffer:
    blk_cleanup_queue(mini2440_ramdisk_devp->queue);
    kfree(mini2440_ramdisk_devp);
    unregister_blkdev(mini2440_ramdisk_major, "ramdisk");
    return ret;
} 

static void __exit mini2440_ramdisk_exit(void)
{
    del_gendisk(mini2440_ramdisk_devp->gdisk);
    put_disk(mini2440_ramdisk_devp->gdisk);
    kfree(mini2440_ramdisk_devp->ramdisk_buffer);
    blk_cleanup_queue(mini2440_ramdisk_devp->queue);
    kfree(mini2440_ramdisk_devp);
    unregister_blkdev(mini2440_ramdisk_major, "mini2440_ramdisk");
}

module_init(mini2440_ramdisk_init);
module_exit(mini2440_ramdisk_exit);

MODULE_LICENSE("GPL");

