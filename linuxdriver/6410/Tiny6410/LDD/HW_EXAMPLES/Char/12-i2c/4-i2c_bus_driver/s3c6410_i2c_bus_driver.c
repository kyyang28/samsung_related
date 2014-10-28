
/*
 *  Purpose:
 *  Allocate, setup and register i2c_adapter structure
 *  
 */
static int __init s3c_i2c_bus_driver_init(void)
{
    /* Register i2c_adapter */
    i2c_add_adapter(struct i2c_adapter * adapter);

    return 0;
}

static void __exit s3c_i2c_bus_driver_exit(void)
{
}

module_init(s3c_i2c_bus_driver_init);
module_exit(s3c_i2c_bus_driver_exit);

MODULE_LICENSE("GPL");

