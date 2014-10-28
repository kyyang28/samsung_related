

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/hid.h>
#include <linux/usb/input.h>
#include <linux/slab.h>             /* kmalloc/kfree */


struct usb_kbd {
	struct input_dev *dev;
	struct usb_device *usbdev;
	unsigned char old[8];
	struct urb *irq;
	unsigned char *new;
	dma_addr_t new_dma;
};




static void tiny6410_usbkbd_irq(struct urb *urb)
{
	struct usb_kbd *kbd = urb->context;
    int i;

	switch (urb->status) {
	case 0:			/* success */
		break;
	case -ECONNRESET:	/* unlink */
	case -ENOENT:
	case -ESHUTDOWN:
		return;
	/* -EPIPE:  should clear the halt */
	default:		/* error */
		goto resubmit;
	}

    for (i = 0; i < 8; ++i)
        input_report_key(kbd->dev, KEY, int value)

resubmit:    
	i = usb_submit_urb (urb, GFP_ATOMIC);
	if (i)
		printk("[DRIVER]Cannot resubmit!\n");
}

static int tiny6410_usbkbd_probe(struct usb_interface *intf,
          const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(intf);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct usb_kbd *kbd;
	struct input_dev *input_dev;
	int i, pipe, maxp;
	int error = -ENOMEM;

	interface = intf->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	kbd = kzalloc(sizeof(struct usb_kbd), GFP_KERNEL);
	input_dev = input_allocate_device();
	if (!kbd || !input_dev)
		goto fail1;

	if (!(kbd->irq = usb_alloc_urb(0, GFP_KERNEL)))
		goto fail2;

	if (!(kbd->new = usb_alloc_coherent(dev, 8, GFP_ATOMIC, &kbd->new_dma)))
		goto fail3;

	kbd->usbdev = dev;
	kbd->dev = input_dev;

	input_set_drvdata(input_dev, kbd);

    /* Setup the keys */
    set_bit(EV_KEY, input_dev->evbit);
    set_bit(EV_REP, input_dev->evbit);

    set_bit(KEY_L, input_dev->keybit);
    set_bit(KEY_S, input_dev->keybit);
    set_bit(KEY_ENTER, input_dev->keybit);

	usb_fill_int_urb(kbd->irq, dev, pipe,
			 kbd->new, (maxp > 8 ? 8 : maxp),
			 tiny6410_usbkbd_irq, kbd, endpoint->bInterval);
	kbd->irq->transfer_dma = kbd->new_dma;
	kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	error = input_register_device(kbd->dev);
	if (error)
		goto fail3;

	usb_set_intfdata(intf, kbd);

    usb_submit_urb(kbd->irq, GFP_KERNEL);
        
    return 0;

fail3:    
	usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
fail2:
    usb_free_urb(kbd->irq);
fail1:    
	input_free_device(input_dev);
	kfree(kbd);
	return error;
}

static void tiny6410_usbkbd_disconnect(struct usb_interface *intf)
{
	struct usb_kbd *kbd = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);

    if (kbd) {
        usb_kill_urb(kbd->irq);
        input_unregister_device(kbd->dev);        
        usb_free_coherent(dev, 8, kbd->new, kbd->new_dma);
        usb_free_urb(kbd->irq);
        kfree(kbd);
    }
}


static struct usb_device_id tiny6410_usbkbd_id_table [] = {
    { USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT, 
        USB_INTERFACE_PROTOCOL_KEYBOARD)},
    { } /* Terminating entry */
};

static struct usb_driver tiny6410_usbkbd_drv = {
    .name               = "tiny6410_usbkbd",
    .probe              = tiny6410_usbkbd_probe,
    .disconnect         = tiny6410_usbkbd_disconnect,
    .id_table           = tiny6410_usbkbd_id_table,
};

static int __init tiny6410_usbkbd_init(void)
{
    int ret = usb_register(&tiny6410_usbkbd_drv);
    if (ret != 0) {
        printk("[DRIVER]Error: failed to register tiny6410_usbkbd_drv!\n");
        return ret;
    }
    return 0;
}

static void __exit tiny6410_usbkbd_exit(void)
{
    usb_deregister(&tiny6410_usbkbd_drv);
}

module_init(tiny6410_usbkbd_init);
module_exit(tiny6410_usbkbd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");

