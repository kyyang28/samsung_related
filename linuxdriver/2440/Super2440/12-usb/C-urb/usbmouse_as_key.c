
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


static struct input_dev *usbmouse_key_dev;
static char *usb_buffer;
static dma_addr_t usb_buffer_phys;
static struct urb *usbmouse_key_urb;

static int len;


static void usbmouse_as_key_irq(struct urb *urb)
{
    int i;
    static int cnt = 0;
    printk("cnt = %d: ", ++cnt);
    for (i = 0; i < len; ++i)
        printk("%02x ", usb_buffer[i]);
    printk("\n");

    /* Re-submit urb */
    usb_submit_urb(usbmouse_key_urb, GFP_KERNEL);
}


static int usbmouse_as_key_probe(struct usb_interface *intf, const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;

    int error = -ENOMEM;
    int pipe;

//    printk("Invoking usbmouse_as_key_probe function....\n");

//    printk("bcdUSB      = %x\n", dev->descriptor.bcdUSB);
//    printk("idVendor    = %x\n", dev->descriptor.idVendor);
//    printk("idProduct   = %x\n", dev->descriptor.idProduct);

	interface = intf->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;

    /*  1. Allocate an input_dev structure */
    usbmouse_key_dev = input_allocate_device();
    
    /*  2. Configure this input_dev structure */
    /*  2.1 Which event will occur */
    set_bit(EV_KEY, usbmouse_key_dev->evbit);
    set_bit(EV_REP, usbmouse_key_dev->evbit);

    /*  2.2 In the specific event, which sub-event will occur */
    set_bit(KEY_L, usbmouse_key_dev->keybit);
    set_bit(KEY_S, usbmouse_key_dev->keybit);
    set_bit(KEY_ENTER, usbmouse_key_dev->keybit);
    
    /*  3. Register */
    error = input_register_device(usbmouse_key_dev);
    if(error) {
        input_free_device(usbmouse_key_dev);
        return error;
    }
    
    /*  4. Hardware specific configurations */
    /*  Focusing on the 3 major key elements of data transfer 
     *  a) Source  b) Destination c) Length
     */
    /* Source: one of the USB device endpoint */
	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);

    /* Length:  */
    len = endpoint->wMaxPacketSize;

    /* Destination:  */
	usb_buffer = usb_alloc_coherent(dev, len, GFP_ATOMIC, &usb_buffer_phys);

    /* Use these 3 major key elements */
    /* Allocate urb ( USB Request Block ) */
	usbmouse_key_urb = usb_alloc_urb(0, GFP_KERNEL);    

    /* Use these 3 major key elements, configure this urb */
	usb_fill_int_urb(usbmouse_key_urb, dev, pipe, usb_buffer,
			 (len > 8 ? 8 : len),
			 usbmouse_as_key_irq, NULL, endpoint->bInterval);
	usbmouse_key_urb->transfer_dma = usb_buffer_phys;
	usbmouse_key_urb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    /* Use URB */
	if (usb_submit_urb(usbmouse_key_urb, GFP_KERNEL))
		return -EIO;
    
    return 0;
}


static void usbmouse_as_key_disconnect(struct usb_interface *intf)
{
    //printk("Invoking usbmouse_as_key_disconnect function....\n");
	usb_kill_urb(usbmouse_key_urb);
    usb_free_urb(usbmouse_key_urb);
    usb_free_coherent(interface_to_usbdev(intf), len, usb_buffer, usb_buffer_phys);
    input_unregister_device(usbmouse_key_dev);
    input_free_device(usbmouse_key_dev);
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



