

#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/usb/input.h>
#include <linux/hid.h>


static const unsigned char tiny6410_usbkbd_keycode[256] = {
	  0,  0,  0,  0, 30, 48, 46, 32, 18, 33, 34, 35, 23, 36, 37, 38,
	 50, 49, 24, 25, 16, 19, 31, 20, 22, 47, 17, 45, 21, 44,  2,  3,
	  4,  5,  6,  7,  8,  9, 10, 11, 28,  1, 14, 15, 57, 12, 13, 26,
	 27, 43, 43, 39, 40, 41, 51, 52, 53, 58, 59, 60, 61, 62, 63, 64,
	 65, 66, 67, 68, 87, 88, 99, 70,119,110,102,104,111,107,109,106,
	105,108,103, 69, 98, 55, 74, 78, 96, 79, 80, 81, 75, 76, 77, 71,
	 72, 73, 82, 83, 86,127,116,117,183,184,185,186,187,188,189,190,
	191,192,193,194,134,138,130,132,128,129,131,137,133,135,136,113,
	115,114,  0,  0,  0,121,  0, 89, 93,124, 92, 94, 95,  0,  0,  0,
	122,123, 90, 91, 85,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,  0,
	 29, 42, 56,125, 97, 54,100,126,164,166,165,163,161,115,114,113,
	150,158,159,128,136,177,178,176,142,152,173,140
};

struct tiny6410_usbkbd {
	struct input_dev *dev;
	struct usb_device *usbdev;
	unsigned char old[8];
	struct urb *irq;
	unsigned char newleds;
	unsigned char *new;
	dma_addr_t new_dma;
	dma_addr_t leds_dma;
};

static void tiny6410_usbkbd_irq(struct urb *urb)
{
	struct tiny6410_usbkbd *kbd = urb->context;
	int i;
    //static int cnt = 0;

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

#if 0
    printk("cnt = %d: ", ++cnt);
    for (i = 0; i < 8; ++i) {
        printk("tiny6410_usbkbd_keycode[%d] = %c, %d, 0x%x\n", i+224, tiny6410_usbkbd_keycode[i + 224], tiny6410_usbkbd_keycode[i + 224], tiny6410_usbkbd_keycode[i + 224]);
        //printk("%02x ", kbd->new[0]);
    }
    printk("\n");
#endif

	for (i = 0; i < 8; i++)
		input_report_key(kbd->dev, tiny6410_usbkbd_keycode[i + 224], (kbd->new[0] >> i) & 1);

	for (i = 2; i < 8; i++) {

		if (kbd->old[i] > 3 && memscan(kbd->new + 2, kbd->old[i], 6) == kbd->new + 8) {
            //printk("\nkbd->old[%d] = %c, %d, 0x%x\n", i, tiny6410_usbkbd_keycode[kbd->old[i]], tiny6410_usbkbd_keycode[kbd->old[i]], tiny6410_usbkbd_keycode[kbd->old[i]]);
			if (tiny6410_usbkbd_keycode[kbd->old[i]])
				input_report_key(kbd->dev, tiny6410_usbkbd_keycode[kbd->old[i]], 0);
			else
				hid_info(urb->dev,
					 "Unknown key (scancode %#x) released.\n",
					 kbd->old[i]);
		}

		if (kbd->new[i] > 3 && memscan(kbd->old + 2, kbd->new[i], 6) == kbd->old + 8) {
            //printk("\nkbd->new[%d] = %c, %d, 0x%x\n", i, tiny6410_usbkbd_keycode[kbd->new[i]], tiny6410_usbkbd_keycode[kbd->new[i]], tiny6410_usbkbd_keycode[kbd->new[i]]);
			if (tiny6410_usbkbd_keycode[kbd->new[i]])
				input_report_key(kbd->dev, tiny6410_usbkbd_keycode[kbd->new[i]], 1);
			else
				hid_info(urb->dev,
					 "Unknown key (scancode %#x) released.\n",
					 kbd->new[i]);
		}
	}

	input_sync(kbd->dev);

	memcpy(kbd->old, kbd->new, 8);

resubmit:
	i = usb_submit_urb (urb, GFP_ATOMIC);
	if (i)
		printk("[DRIVER]Cannot resubmit!\n");
}

static int tiny6410_usbkbd_probe(struct usb_interface *iface,
			 const struct usb_device_id *id)
{
	struct usb_device *dev = interface_to_usbdev(iface);
	struct usb_host_interface *interface;
	struct usb_endpoint_descriptor *endpoint;
	struct tiny6410_usbkbd *kbd;
	struct input_dev *input_dev;
	int i, pipe, maxp;
	int error = -ENOMEM;

	interface = iface->cur_altsetting;

	if (interface->desc.bNumEndpoints != 1)
		return -ENODEV;

	endpoint = &interface->endpoint[0].desc;
	if (!usb_endpoint_is_int_in(endpoint))
		return -ENODEV;

	pipe = usb_rcvintpipe(dev, endpoint->bEndpointAddress);
	maxp = usb_maxpacket(dev, pipe, usb_pipeout(pipe));

	kbd = kzalloc(sizeof(struct tiny6410_usbkbd), GFP_KERNEL);
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

	input_dev->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_LED) |
		BIT_MASK(EV_REP);
	input_dev->ledbit[0] = BIT_MASK(LED_NUML) | BIT_MASK(LED_CAPSL) |
		BIT_MASK(LED_SCROLLL) | BIT_MASK(LED_COMPOSE) |
		BIT_MASK(LED_KANA);

	for (i = 0; i < 255; i++)
		set_bit(tiny6410_usbkbd_keycode[i], input_dev->keybit);
	clear_bit(0, input_dev->keybit);

	usb_fill_int_urb(kbd->irq, dev, pipe,
			 kbd->new, (maxp > 8 ? 8 : maxp),
			 tiny6410_usbkbd_irq, kbd, endpoint->bInterval);
	kbd->irq->transfer_dma = kbd->new_dma;
	kbd->irq->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

	error = input_register_device(kbd->dev);
	if (error)
		goto fail2;

	usb_set_intfdata(iface, kbd);

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
	struct tiny6410_usbkbd *kbd = usb_get_intfdata (intf);

	usb_set_intfdata(intf, NULL);
	if (kbd) {
		usb_kill_urb(kbd->irq);
		input_unregister_device(kbd->dev);
        usb_free_coherent(kbd->usbdev, 8, kbd->new, kbd->new_dma);
        usb_free_urb(kbd->irq);
		kfree(kbd);
	}
}

static struct usb_device_id tiny6410_usbkbd_id_table [] = {
	{ USB_INTERFACE_INFO(USB_INTERFACE_CLASS_HID, USB_INTERFACE_SUBCLASS_BOOT,
		USB_INTERFACE_PROTOCOL_KEYBOARD) },
	{ }	/* Terminating entry */
};


static struct usb_driver tiny6410_usbkbd_driver = {
	.name               = "tiny6410_usbkbd",
	.probe              = tiny6410_usbkbd_probe,
	.disconnect         = tiny6410_usbkbd_disconnect,
	.id_table           = tiny6410_usbkbd_id_table,
};

static int __init tiny6410_usbkbd_init(void)
{
	int ret = usb_register(&tiny6410_usbkbd_driver);
	if (ret != 0) {
        printk("[DRIVER]Error: failed to register tiny6410_usbmouse_drv!\n");
        return ret;
	}
	return ret;
}

static void __exit tiny6410_usbkbd_exit(void)
{
	usb_deregister(&tiny6410_usbkbd_driver);
}

module_init(tiny6410_usbkbd_init);
module_exit(tiny6410_usbkbd_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


