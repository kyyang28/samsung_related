

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/init.h>
#include <linux/errno.h>

//#define AT24CXX_DBG  printk
#define AT24CXX_DBG(...)

struct at24cxx_i2c {
    struct cdev cdev;
    struct class *cls;
    struct i2c_client *client;
    unsigned char addr;
    unsigned char data;
    unsigned char kernel_buf[2];
};

static int at24cxx_driver_major;
static struct at24cxx_i2c *at24cxx_driverp;

static int at24cxx_open(struct inode *inode, struct file *filp)
{
    filp->private_data = at24cxx_driverp;
    return 0;
}

/* 
 *  input:   buf[0] = addr
 *  output:  buf[0] = data
 */
static ssize_t at24cxx_read(struct file *filp, char __user *buf, size_t count, 
        loff_t *ppos)
{
    struct at24cxx_i2c *drv = filp->private_data;

    if (copy_from_user(&drv->addr, buf, 1))
        return -EFAULT;

    AT24CXX_DBG("[at24cxx_driver]addr is %u\n", drv->addr);

    drv->data = i2c_smbus_read_byte_data(drv->client, drv->addr);

    AT24CXX_DBG("[at24cxx_driver]data is %u\n", drv->data);

    if (copy_to_user(buf, &drv->data, 1))
        return -EFAULT;
    
    return 1;
}

/* @param buf[0] = addr, buf[1] = data */
static ssize_t at24cxx_write(struct file *filp, const char __user *buf, size_t count, 
        loff_t *ppos)
{
    struct at24cxx_i2c *drv = filp->private_data;

    if (copy_from_user(drv->kernel_buf, buf, 2))
        return -EFAULT;

    AT24CXX_DBG("[at24cxx_driver]kernel_buf[0] = %u\n", (drv->kernel_buf)[0]);
    AT24CXX_DBG("[at24cxx_driver]kernel_buf[1] = %u\n", (drv->kernel_buf)[1]);

    drv->addr = (drv->kernel_buf)[0];
    drv->data = (drv->kernel_buf)[1];

	AT24CXX_DBG("[at24cxx_driver]addr = 0x%02x, data = 0x%02x\n", drv->addr, drv->data);

    if (!i2c_smbus_write_byte_data(drv->client, drv->addr, drv->data))
        return 2;
    else 
        return -EIO;
}

static struct file_operations at24cxx_fops = {
    .owner      = THIS_MODULE,
    .open       = at24cxx_open,
    .read       = at24cxx_read,
    .write      = at24cxx_write,
};


static const struct i2c_device_id at24cxx_id_table[] = {
	{ "at24c08", 0 },
};

static int at24cxx_driver_rest_setup(struct at24cxx_i2c *drv, 
        struct i2c_client *client, int minor)
{
    int ret = 0;
    int err, devno = MKDEV(at24cxx_driver_major, minor);

    drv->client = client;

    cdev_init(&drv->cdev, &at24cxx_fops);
    drv->cdev.owner = THIS_MODULE;
    err = cdev_add(&drv->cdev, devno, 1);
    if (err) {
        printk(KERN_NOTICE "[At24cxx_driver]Error %d adding at24cxx %d", err, minor);
        ret = -ENODEV;
        return ret;
    }
        
    drv->cls = class_create(THIS_MODULE, "at24cxx");    
	if (IS_ERR(drv->cls)) {
        printk(KERN_NOTICE "[At24cxx_driver]Error: creating class\n");
        cdev_del(&drv->cdev);
		ret = -ENOMEM;
        return ret;
	}
    
    device_create(drv->cls, NULL, MKDEV(at24cxx_driver_major, 0), NULL, "at24c08"); /* /dev/at24c08 */    
    
    memset(drv->kernel_buf, 0, sizeof(drv->kernel_buf));

    /* Save at24cxx_driver structure to the atapter's algo_data */
    //drv->client->adapter->algo_data = drv;  /* Adding this line gets error, why? */
    return ret;
}

static int __devinit at24cxx_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
    //printk("[at24cxx_driver.c]In function %s, at line %d\n", __FUNCTION__, __LINE__);
    int result;
    dev_t devno = MKDEV(at24cxx_driver_major, 0);

    /* register device number */
    if (at24cxx_driver_major)
        result = register_chrdev_region(devno, 1, "at24c08");
    else {
        result = alloc_chrdev_region(&devno, 0, 1, "at24c08");
        at24cxx_driver_major = MAJOR(devno);
    }

    if (result < 0)
        return result;

    at24cxx_driverp = kmalloc(sizeof(struct at24cxx_i2c), GFP_KERNEL);
    if (!at24cxx_driverp) {
        result = -ENOMEM;
        goto fail_malloc;
    }

    memset(at24cxx_driverp, 0, sizeof(struct at24cxx_i2c));

    result = at24cxx_driver_rest_setup(at24cxx_driverp, client, 0);
    if (result < 0)
        goto fail_setup;
    
    return 0;

fail_malloc:
    unregister_chrdev_region(devno, 1);
    return result;

fail_setup:
    kfree(at24cxx_driverp);
    unregister_chrdev_region(devno, 1);
    return result;
}

static int __devexit at24cxx_remove(struct i2c_client *client)
{
    //printk("[at24cxx_driver.c]In function %s, at line %d\n", __FUNCTION__, __LINE__);
    device_destroy(at24cxx_driverp->cls, MKDEV(at24cxx_driver_major, 0));
    class_destroy(at24cxx_driverp->cls);
    cdev_del(&at24cxx_driverp->cdev);
    kfree(at24cxx_driverp);
    unregister_chrdev_region(MKDEV(at24cxx_driver_major, 0), 1);
    return 0;
}

static struct i2c_driver at24cxx_driver = {
	.driver	= {
		.name	= "at24cxx",
		.owner	= THIS_MODULE,
	},
	.probe		= at24cxx_probe,
	.remove		= __devexit_p(at24cxx_remove),
	.id_table	= at24cxx_id_table,
};


static int __init at24cxx_driver_init(void)
{
    /* Register i2c_driver structure */
	int ret;
	ret = i2c_add_driver(&at24cxx_driver);
	if (ret != 0)
		pr_err("[at24cxx_driver]Failed to register at24cxx I2C driver: %d\n", ret);
	return ret;
}

static void __exit at24cxx_driver_exit(void)
{
    i2c_del_driver(&at24cxx_driver);
}

module_init(at24cxx_driver_init);
module_exit(at24cxx_driver_exit);

MODULE_LICENSE("GPL");

