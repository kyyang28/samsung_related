

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/platform_device.h>
#include <linux/i2c.h>
#include <linux/slab.h>


static struct i2c_client *at24cxx_client;

static struct i2c_board_info at24cxx_info = {
	I2C_BOARD_INFO("at24c08", 0x50),    /* 0x50 is the device address */
};

static int __init at24cxx_device_init(void)
{
    int ret = 0;
    struct i2c_adapter *i2c_adap;
    
    /* 
     *  Get the i2c_adapter using i2c_get_adapter(nr)
     *  Change the number(nr) accordingly
     *  e.g. if /dev/i2c/0, then use i2c_get_adapter(0)
     *       if /dev/i2c/1, then use i2c_get_adapter(1)
     *       etc.
     */
    i2c_adap = i2c_get_adapter(1);
    
    /* Using i2c_new_device explicitly */
    at24cxx_client = i2c_new_device(i2c_adap, &at24cxx_info);

    /* Put the i2c_adapter */
    i2c_put_adapter(i2c_adap);

    if (!at24cxx_client) {
        ret = -ENODEV;
    }
    
    return ret;
}

static void __exit at24cxx_device_exit(void)
{
    i2c_unregister_device(at24cxx_client);
}

module_init(at24cxx_device_init);
module_exit(at24cxx_device_exit);

MODULE_LICENSE("GPL");

