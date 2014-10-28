

#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/gpio.h>
#include <linux/poll.h>
#include <linux/types.h>
#include <linux/cdev.h>
#include <linux/io.h>
#include <linux/ioport.h>
#include <linux/mutex.h>
#include <linux/nsc_gpio.h>
#include <linux/platform_device.h>


#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>


#define CHRDEV_ANOTHER_CNT         2


/* 1. Declare major device nunber */
static int major;

static struct cdev chrdev_another_cdev;
static struct cdev chrdev_another2_cdev;

static struct class *chrdev_another_class;
static struct device *chrdev_another_dev;

static int chrdev_another_open(struct inode *inode, struct file *file)
{
    printk("chrdev_another_open is invoking!\n");
    return 0;
}

static int chrdev_another2_open(struct inode *inode, struct file *file)
{
    printk("chrdev_another_open2 is invoking!\n");
    return 0;
}


/* 2. Construct file_operations struct */
static struct file_operations chrdev_another_fops = {
	.owner   	= THIS_MODULE,
	.open    	= chrdev_another_open,
};

static struct file_operations chrdev_another2_fops = {
	.owner   	= THIS_MODULE,
	.open    	= chrdev_another2_open,
};

static int __init chrdev_another_init(void)
{
    dev_t devid;
    //major = register_chrdev_region(0, "chrdev_another", &chrdev_another_fops);

    if (major) {
        devid = MKDEV(major, 0);
        /*  (major,0) corresponds to chrdev_another_fops 
         *  (major,1~255) corresponds to other fops
         */
        register_chrdev_region(devid, CHRDEV_ANOTHER_CNT, "chrdev_another"); 
    } else {
        alloc_chrdev_region(&devid, 0, CHRDEV_ANOTHER_CNT, "chrdev_another");
        major = MAJOR(devid);
    }
    
	cdev_init(&chrdev_another_cdev, &chrdev_another_fops);
	cdev_add(&chrdev_another_cdev, devid, CHRDEV_ANOTHER_CNT);

    devid = MKDEV(major, 2);
    register_chrdev_region(devid, 1, "chrdev_another2"); 
	cdev_init(&chrdev_another2_cdev, &chrdev_another2_fops);
	cdev_add(&chrdev_another2_cdev, devid, 1);


    chrdev_another_class = class_create(THIS_MODULE, "chrdev_another");
	if (IS_ERR(chrdev_another_class)) {
        cdev_del(&chrdev_another_cdev);
    	unregister_chrdev_region(MKDEV(major,0), CHRDEV_ANOTHER_CNT);
		return PTR_ERR(chrdev_another_class);
	}

	chrdev_another_dev = device_create(chrdev_another_class, NULL, MKDEV(major, 0), NULL, "chrdevAnother0"); 
	if (IS_ERR(chrdev_another_dev)) {
        class_destroy(chrdev_another_class);
		cdev_del(&chrdev_another_cdev);
    	unregister_chrdev_region(MKDEV(major,0), CHRDEV_ANOTHER_CNT);
        return PTR_ERR(chrdev_another_dev);
    }

	chrdev_another_dev = device_create(chrdev_another_class, NULL, MKDEV(major, 1), NULL, "chrdevAnother1"); 
	if (IS_ERR(chrdev_another_dev)) {
        device_destroy(chrdev_another_class, MKDEV(major, 0));
        class_destroy(chrdev_another_class);
		cdev_del(&chrdev_another_cdev);
    	unregister_chrdev_region(MKDEV(major,1), CHRDEV_ANOTHER_CNT);
        return PTR_ERR(chrdev_another_dev);
    }

	chrdev_another_dev = device_create(chrdev_another_class, NULL, MKDEV(major, 2), NULL, "chrdevAnother2"); 
	if (IS_ERR(chrdev_another_dev)) {
        device_destroy(chrdev_another_class, MKDEV(major, 0));
        device_destroy(chrdev_another_class, MKDEV(major, 1));
        class_destroy(chrdev_another_class);
		cdev_del(&chrdev_another_cdev);
    	unregister_chrdev_region(MKDEV(major,2), CHRDEV_ANOTHER_CNT);
        return PTR_ERR(chrdev_another_dev);
    }

	chrdev_another_dev = device_create(chrdev_another_class, NULL, MKDEV(major, 3), NULL, "chrdevAnother3"); 
	if (IS_ERR(chrdev_another_dev)) {
        device_destroy(chrdev_another_class, MKDEV(major, 0));
        device_destroy(chrdev_another_class, MKDEV(major, 1));
        device_destroy(chrdev_another_class, MKDEV(major, 2));
        class_destroy(chrdev_another_class);
		cdev_del(&chrdev_another_cdev);
    	unregister_chrdev_region(MKDEV(major,3), 1);
        return PTR_ERR(chrdev_another_dev);
    }


    return 0;
}


static void __exit chrdev_another_exit(void)
{
    device_destroy(chrdev_another_class, MKDEV(major, 0));
    device_destroy(chrdev_another_class, MKDEV(major, 1));
    device_destroy(chrdev_another_class, MKDEV(major, 2));
    device_destroy(chrdev_another_class, MKDEV(major, 3));
    class_destroy(chrdev_another_class);
    cdev_del(&chrdev_another_cdev);
	unregister_chrdev_region(MKDEV(major,0), CHRDEV_ANOTHER_CNT);
    cdev_del(&chrdev_another2_cdev);
	unregister_chrdev_region(MKDEV(major,2), 1);
}


module_init(chrdev_another_init);
module_exit(chrdev_another_exit);

MODULE_LICENSE("Dual BSD/GPL");


