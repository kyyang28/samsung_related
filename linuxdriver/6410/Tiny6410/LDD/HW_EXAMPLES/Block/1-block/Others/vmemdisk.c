
#include <linux/module.h>               
#include <linux/moduleparam.h>          /* module_param */
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


#define VMEMDISK_MINOR              16
#define KERNEL_SECTOR_SIZE          512
#define INVALIDATE_DELAY            30*HZ


static int vmemdisk_major = 0;
module_param(vmemdisk_major, int, 0);
static int ndevices = 4;
module_param(ndevices, int, 0);
static int hardsect_size = 512;
module_param(hardsect_size, int, 0);
static int nsectors = 1024;
module_param(nsectors, int, 0);


struct vmemdisk_dev {
    int size;                       /* The device size in sectors */
    u8 *data;                       /* The data array */
    short users;                    /* How many users */
    short media_change;             /* Media change flag? */
    spinlock_t lock;                /* For mutual exclusion */
    struct request_queue *queue;    /* The device request queue */
    struct gendisk *gd;             /* The gendisk structure */
    struct timer_list timer;        /* For the simulated media change */    
};

static struct vmemdisk_dev *vmemdisk_devices;

enum {
    RM_SIMPLE   = 0,
    RM_FULL     = 1,
    RM_NOQUEUE  = 2,
};

static int request_mode = RM_SIMPLE;
module_param(request_mode, int, 0);


static int vmemdisk_open(struct block_device *bdev, fmode_t mode)
{
    struct vmemdisk_dev *dev = bdev->bd_disk->private_data;

    del_timer_sync(&dev->timer);
    spin_lock(&dev->lock);
    dev->users++;
    spin_unlock(&dev->lock);

    return 0;
}

static int vmemdisk_release(struct gendisk *gdisk, fmode_t mode)
{
    struct vmemdisk_dev *dev = gdisk->private_data;

    spin_lock(&dev->lock);
    dev->users--;
    if (!dev->users) {
        dev->timer.expires = jiffies + INVALIDATE_DELAY;
        add_timer(&dev->timer);
    }
    spin_unlock(&dev->lock);

    return 0;
}

static int vmemdisk_media_changed(struct gendisk *gdisk)
{
    struct vmemdisk_dev *dev = gdisk->private_data;
    return dev->media_change;
}

static int vmemdisk_revalidate(struct gendisk *gdisk)
{
    struct vmemdisk_dev *dev = gdisk->private_data;

    if (dev->media_change) {
        dev->media_change = 0;
        memset(dev->data, 0, dev->size);
    }
    
    return 0;
}


static int vmemdisk_getgeo(struct block_device *bdev, struct hd_geometry *geo)
{
    geo->heads = 4;
    geo->cylinders = 16;
    geo->sectors = 16;
    
    return 0;
}


static struct block_device_operations vmemdisk_fops = {
    .owner              = THIS_MODULE,
    .open               = vmemdisk_open,
    .release            = vmemdisk_release,
    .media_changed      = vmemdisk_media_changed,
    .revalidate_disk    = vmemdisk_revalidate,
    .getgeo             = vmemdisk_getgeo,
};

static void vmemdisk_invalidate(unsigned long ldev)
{
    struct vmemdisk_dev *dev = (struct vmemdisk_dev *)ldev;

    spin_lock(&dev->lock);

    if (dev->users || !dev->data)
        printk(KERN_WARNING "[DRIVER]vmemdisk: timer checks failed!\n");
    else 
        dev->media_change = 1;

    spin_unlock(&dev->lock);
}


static void vmemdisk_transfer(struct vmemdisk_dev *dev, unsigned long sector, 
    unsigned long nsect, char *buffer, int write)
{
    unsigned long offset = sector * KERNEL_SECTOR_SIZE;
    unsigned long nbytes = nsect * KERNEL_SECTOR_SIZE;

    if ((offset + nbytes) > dev->size) {
        printk(KERN_NOTICE "[DRIVER]Beyond-end write (%ld %ld)!\n", offset, nbytes);
        return;
    }

    if (write)
        memcpy(dev->data + offset, buffer, nbytes);
    else
        memcpy(buffer, dev->data + offset, nbytes);
}


/* Helper method to transfer a single BIO */
static int vmemdisk_xfer_bio(struct vmemdisk_dev *dev, struct bio *bio)
{
    int i;
    struct bio_vec *bvec;
    sector_t sector = bio->bi_sector;

    /* Do each segment independently */
    bio_for_each_segment(bvec, bio, i) {
        char *buffer = __bio_kmap_atomic(bio, i, KM_USER0);
        vmemdisk_transfer(dev, sector, (bio_cur_bytes(bio)>>9), buffer, bio_data_dir(bio) == WRITE);
        sector += (bio_cur_bytes(bio)>>9);
        __bio_kunmap_atomic(bio, KM_USER0);
    }
    
    return 0;
}


/* Helper method to make request */
static int vmemdisk_make_request(struct request_queue *q, struct bio *bio)
{
    int status;
    struct vmemdisk_dev *dev = q->queuedata;

    status = vmemdisk_xfer_bio(dev, bio);
    bio_endio(bio, status);

    return 0;
}

static int vmemdisk_xfer_request(struct vmemdisk_dev *dev, struct request *req)
{
    struct req_iterator iter;
    int nsect = 0;
    struct bio_vec *bvec;

    rq_for_each_segment(bvec, req, iter) {
        char *buffer = __bio_kmap_atomic(iter.bio, iter.i, KM_USER0);
        sector_t sector = iter.bio->bi_sector;
        vmemdisk_transfer(dev, sector, (bio_cur_bytes(iter.bio)>>9), buffer, 
            bio_data_dir(iter.bio) == WRITE);
        sector += (bio_cur_bytes(iter.bio)>>9);
        __bio_kunmap_atomic(iter.bio, KM_USER0);
        nsect += iter.bio->bi_size / KERNEL_SECTOR_SIZE;
    }

    return nsect;
}

static void vmemdisk_full_request(struct request_queue *q)
{
    struct request *req;
    int sectors_xferred;
    struct vmemdisk_dev *dev = q->queuedata;

    while ((req = blk_fetch_request(q)) != NULL) {
        sectors_xferred = vmemdisk_xfer_request(dev, req);
        __blk_end_request_cur(req, 1);
    } 
}

static void vmemdisk_simple_request(struct request_queue *q)
{
    struct request *req;

    while ((req = blk_fetch_request(q)) != NULL) {
        struct vmemdisk_dev *dev = req->rq_disk->private_data;
        vmemdisk_transfer(dev, req->__sector, req->__data_len, req->buffer, 
            rq_data_dir(req));
        __blk_end_request_cur(req, 1);
    }
}


/* Helper method to setup the block device */
static void setup_vmemdisk_device(struct vmemdisk_dev *dev, int which)
{
    memset(dev, 0, sizeof(struct vmemdisk_dev));
    dev->size = nsectors * hardsect_size;
    dev->data = vmalloc(dev->size);
    if (!dev->data) {
        printk(KERN_NOTICE "vmalloc failure!\n");
        return;
    }

    /*  Initialize spinlock */
    spin_lock_init(&dev->lock);

    /*  Use the timer to simulate the invalidate device */
    init_timer(&dev->timer);
    dev->timer.data = (unsigned long)dev;
    dev->timer.function = vmemdisk_invalidate;
    
    /*  
     *  I/O queue 
     *  specific implementation depends on whether we use make_request function or not 
     */
    switch (request_mode) {
    case RM_NOQUEUE:
        dev->queue = blk_alloc_queue(GFP_KERNEL);
        if (!dev->queue)
            goto out;

        blk_queue_make_request(dev->queue, vmemdisk_make_request);
        break;

    case RM_FULL:
        dev->queue = blk_init_queue(vmemdisk_full_request, &dev->lock);
        if (!dev->queue)
            goto out;
        break;

    case RM_SIMPLE:
        dev->queue = blk_init_queue(vmemdisk_simple_request, &dev->lock);
        if (!dev->queue)
            goto out;
        break;

    default:
        printk(KERN_NOTICE "[DRIVER]Bad request mode %d, using RM_SIMPLE!\n", request_mode);
    }

    /* Tell the kernel what the hardware block sector size is */
    blk_queue_logical_block_size(dev->queue, hardsect_size);

    /* [IMPORTANT] store the object dev to the dev->queue->queuedata for future use */
    dev->queue->queuedata = dev;

    /* Allocate gendisk and initialize it */
    dev->gd = alloc_disk(VMEMDISK_MINOR);
    if (!dev->gd) {
        printk(KERN_NOTICE "alloc_disk failure!\n");
        goto out;
    }

    dev->gd->major          = vmemdisk_major;
    dev->gd->first_minor    = which * VMEMDISK_MINOR;
    dev->gd->fops           = &vmemdisk_fops;
    dev->gd->queue          = dev->queue;
    dev->gd->private_data   = dev;

    snprintf(dev->gd->disk_name, 32, "vmemdisk%c", which + 'a');
    set_capacity(dev->gd, nsectors * (hardsect_size / KERNEL_SECTOR_SIZE));
    add_disk(dev->gd);
    return;

out:
    if (dev->data)
        vfree(dev->data);
}

static int __init vmemdisk_init(void)
{
    int i;
    
    /* Register block devices */
    vmemdisk_major = register_blkdev(vmemdisk_major, "vmemdisk");
    if (vmemdisk_major <= 0) {
        printk(KERN_WARNING "vmemdisk: unable to get major number!\n");
        return -EBUSY;
    }
    
    /* Allocating device arrays and initialize them */
    vmemdisk_devices = kmalloc(ndevices*sizeof(struct vmemdisk_dev), GFP_KERNEL);
    if (!vmemdisk_devices)
        goto out;

    for (i = 0; i < ndevices; ++i)
        setup_vmemdisk_device(vmemdisk_devices + i, i);
    
    return 0;

out:
    unregister_blkdev(vmemdisk_major, "vmemdisk");
    return -ENOMEM;
}

static void __exit vmemdisk_exit(void)
{
    int i;

    /* Clean gendisk, request_queue, data resources for each device */
    for (i = 0; i < ndevices; ++i) {
        struct vmemdisk_dev *dev = vmemdisk_devices + i;
        del_timer_sync(&dev->timer);
        if (dev->gd) {
            del_gendisk(dev->gd);
            put_disk(dev->gd);
        }

        if (dev->queue) {
            if (request_mode == RM_NOQUEUE)
                kobject_put(&dev->queue->kobj);
            else
                blk_cleanup_queue(dev->queue);
        }

        if (dev->data)
            vfree(dev->data);
    }

    /* unregister block device */
    unregister_blkdev(vmemdisk_major, "vmemdisk");
    kfree(vmemdisk_devices);
}

module_init(vmemdisk_init);
module_exit(vmemdisk_exit);

MODULE_LICENSE("GPL");

