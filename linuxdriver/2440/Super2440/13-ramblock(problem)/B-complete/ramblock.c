
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
//#include <linux/zorro.h>

#include <asm/system.h>
#include <asm/uaccess.h>
#include <asm/dma.h>


#define RAMBLOCK_SIZE   (1024*1024)   /* 1M */

#define Z2RAM_CHUNKSIZE		(0x00010000)
#define Z2RAM_CHUNKMASK		(0x0000ffff)
#define Z2RAM_CHUNKSHIFT	(16)


static struct gendisk *ramblock_disk;
static struct request_queue *ramblock_queue;
static unsigned char *ramblock_buffer;

static int major;

static const struct block_device_operations ramblock_fops = {
	.owner	= THIS_MODULE,
//	.ioctl	= ramblock_ioctl,
//	.getgeo = ramblock_getgeo,
};

static u_long *z2ram_map    = NULL;
static u_long z2ram_size    = 0;


static DEFINE_SPINLOCK(ramblock_lock);


static void do_ramblock_request (struct request_queue * q)
{
    //static int cnt = 0;
	struct request *req;

    //printk("do_ramblock_request: cnt = %d\n", ++cnt);

#if 1
    req = blk_fetch_request(q);
    while (req) {
		unsigned long start = blk_rq_pos(req) << 9;
		unsigned long len  = blk_rq_cur_bytes(req);
		int err = 0;

		if (start + len > z2ram_size) {
			err = -EIO;
			goto done;
		}
		while (len) {
			unsigned long addr = start & Z2RAM_CHUNKMASK;
			unsigned long size = Z2RAM_CHUNKSIZE - addr;
			if (len < size)
				size = len;
			addr += z2ram_map[ start >> Z2RAM_CHUNKSHIFT ];
			if (rq_data_dir(req) == READ)
				memcpy(req->buffer, (char *)addr, size);
			else
				memcpy((char *)addr, req->buffer, size);
			start += size;
			len -= size;
		}
	done:
		if (!__blk_end_request_cur(req, err))
			req = blk_fetch_request(q);
	}
#endif
#if 0
	while ((req = blk_fetch_request(q)) != NULL) {
        /* 3 major elements of data transfer */
        /* 3.1 Source: ramblock_buffer+offset */
		unsigned long offset = blk_rq_pos(req) * 512;  /* blk_rq_pos(req) << 9 means blk_rq_pos(req) * 512 */

        /* 3.2 Destination: req->buffer */
        
        /* 3.3 Length */
        unsigned long len    = blk_rq_cur_bytes(req);

	    if (rq_data_dir(req) == READ)
            memcpy(req->buffer, ramblock_buffer+offset, len);
        else
            memcpy(ramblock_buffer+offset, req->buffer, len);


        __blk_end_request_cur(req, 0);
    }
#endif
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

    /* 3. Hardware related operations */
    ramblock_buffer = kzalloc(RAMBLOCK_SIZE, GFP_KERNEL);

    /* 4. Register */
    add_disk(ramblock_disk);
    
    return 0;
}


static void __exit ramblock_exit(void)
{
    unregister_blkdev(major, "ramblock");
    del_gendisk(ramblock_disk);
    put_disk(ramblock_disk);
    blk_cleanup_queue(ramblock_queue);
    kfree(ramblock_buffer);
}


module_init(ramblock_init);
module_exit(ramblock_exit);

MODULE_LICENSE("Dual BSD/GPL");


