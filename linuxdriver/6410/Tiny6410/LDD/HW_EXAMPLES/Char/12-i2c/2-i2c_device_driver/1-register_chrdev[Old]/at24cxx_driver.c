

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>
#include <asm/uaccess.h>
#include <linux/fs.h>


static int major;
static struct class *at24cxx_cls;
static struct i2c_client *at24cxx_client;

/* 
 *  input:   buf[0] = addr
 *  output:  buf[0] = data
 */
static ssize_t at24cxx_read(struct file *filp, char __user *buf, size_t count, 
        loff_t *ppos)
{
    unsigned char addr, data;
    if (copy_from_user(&addr, buf, 1))
        return -EFAULT;

    data = i2c_smbus_read_byte_data(at24cxx_client, addr);

    if (copy_to_user(buf, &data, 1))
        return -EFAULT;
    
    return 1;
}

/* @param buf[0] = addr, buf[1] = data */
static ssize_t at24cxx_write(struct file *filp, const char __user *buf, size_t count, 
        loff_t *ppos)
{
    unsigned char kernel_buf[2];
    unsigned char addr, data;

    if (copy_from_user(kernel_buf, buf, 2))
        return -EFAULT;

    addr = kernel_buf[0];
    data = kernel_buf[1];

	printk("[at24cxx_driver]addr = 0x%02x, data = 0x%02x\n", addr, data);

    if (!i2c_smbus_write_byte_data(at24cxx_client, addr, data))
        return 2;
    else 
        return -EIO;
}

static struct file_operations at24cxx_fops = {
    .owner      = THIS_MODULE,
    .read       = at24cxx_read,
    .write      = at24cxx_write,
};


static const struct i2c_device_id at24cxx_id_table[] = {
	{ "at24c08", 0 },
};

static int __devinit at24cxx_probe(struct i2c_client *client,
				  const struct i2c_device_id *id)
{
    //printk("[at24cxx_driver.c]In function %s, at line %d\n", __FUNCTION__, __LINE__);

    at24cxx_client = client;

    major = register_chrdev(0, "at24cxx", &at24cxx_fops);
    at24cxx_cls = class_create(THIS_MODULE, "at24cxx");
    device_create(at24cxx_cls, NULL, MKDEV(major, 0), NULL, "at24c08"); /* /dev/at24c08 */
    return 0;
}

static int __devexit at24cxx_remove(struct i2c_client *client)
{
    //printk("[at24cxx_driver.c]In function %s, at line %d\n", __FUNCTION__, __LINE__);
    device_destroy(at24cxx_cls, MKDEV(major, 0));
    class_destroy(at24cxx_cls);
    unregister_chrdev(major, "at24cxx");
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

