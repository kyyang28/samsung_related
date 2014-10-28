
/* 
 *  Referenced by linux-2.6.38\drivers\hid\usbhid\Usbmouse.c
 */

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>


/*
 * Version Information
 */
#define DRIVER_VERSION "v1.0"
#define DRIVER_AUTHOR "Charles Yang <charlesyang28@gmail.com>"
#define DRIVER_DESC "USB mouse as key_button driver"


static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
    printk("Invoking usbmouse_as_key_probe function....\n");

	struct usb_device *dev = interface_to_usbdev(intf);

    printk("bcdUSB      = %x\n", dev->descriptor.bcdUSB);
    printk("idVendor    = %x\n", dev->descriptor.idVendor);
    printk("idProduct   = %x\n", dev->descriptor.idProduct);
    
    return 0;
}


static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
    printk("Invoking usbmouse_as_key_disconnect function....\n");    
}


static struct usb_device_id usbmouse_as_key_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_MOUSE) },
	//{ USB_DEVICE(0x1234, 0x5678) },
	{ }	/* Terminating entry */
};


static struct usb_driver usbmouse_as_key_driver = {
	.name		= "usbmouse_as_key",
	.probe		= usbmouse_as_key_probe,
	.disconnect	= usbmouse_as_key_disconnect,
	.id_table	= usbmouse_as_key_id_table,
};


static int __init usbmouse_as_key_init(void)
{
	int retval = usb_register(&usbmouse_as_key_driver);
	if (retval == 0)
		printk(KERN_INFO KBUILD_MODNAME ": " DRIVER_VERSION ":"
				DRIVER_DESC "\n");
	return retval;
}

static void __exit usbmouse_as_key_exit(void)
{
    usb_deregister(&usbmouse_as_key_driver);
}

module_init(usbmouse_as_key_init);
module_exit(usbmouse_as_key_exit);

MODULE_LICENSE("Dual BSD/GPL");



