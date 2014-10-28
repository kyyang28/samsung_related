

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <linux/hid.h>


#define USBMOUSE_PRK            printk
//#define USBMOUSE_PRK(...)

static int tiny6410_usbmouse_probe(struct usb_interface *intf,
          const struct usb_device_id *id)
{
    USBMOUSE_PRK("[DRIVER]tiny6410_usbmouse_probe is invoked!\n");

    return 0;
}

static void tiny6410_usbmouse_disconnect(struct usb_interface *intf)
{
    USBMOUSE_PRK("[DRIVER]tiny6410_usbmouse_disconnect is invoked!\n");
}


static struct usb_device_id tiny6410_usbmouse_id_table [] = {
    { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, 
        USB_INTERFACE_PROTOCOL_MOUSE)},
    { } /* Terminating entry */
};

static struct usb_driver tiny6410_usbmouse_drv = {
    .name               = "tiny6410_usbmouse",
    .probe              = tiny6410_usbmouse_probe,
    .disconnect         = tiny6410_usbmouse_disconnect,
    .id_table           = tiny6410_usbmouse_id_table,
};

static int __init tiny6410_usbmouse_init(void)
{
    int ret = usb_register(&tiny6410_usbmouse_drv);
    if (ret != 0) {
        printk("[DRIVER]Error: failed to register tiny6410_usbmouse_drv!\n");
        return ret;
    }
    return 0;
}

static void __exit tiny6410_usbmouse_exit(void)
{
    usb_deregister(&tiny6410_usbmouse_drv);
}

module_init(tiny6410_usbmouse_init);
module_exit(tiny6410_usbmouse_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


