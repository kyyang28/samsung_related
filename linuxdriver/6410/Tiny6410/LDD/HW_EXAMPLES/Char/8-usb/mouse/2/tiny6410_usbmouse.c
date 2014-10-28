

#include <linux/module.h>
#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/slab.h>
#include <linux/usb/input.h>
#include <linux/hid.h>


struct tiny6410_usbmouse {
	struct usb_device *usbdev;
	struct input_dev *inputdev;
	struct urb *irqurb;
	signed char *data;
	dma_addr_t data_dma;
};


static void tiny6410_usbmouse_irq_complete(struct urb *urb)
{
    struct tiny6410_usbmouse *usbmouse = urb->context;
    signed char *data = usbmouse->data;
    struct input_dev *inputdev = usbmouse->inputdev;
    int status;

    switch (urb->status) {
    case 0:             /* success */
        break;
        
    case -ECONNRESET:   /* unlink */
    case -ENOENT:
    case -ESHUTDOWN:
        return;

    default:            /* error */
        goto resubmit;    
    }

    /* Report key */
    input_report_key(inputdev, BTN_LEFT,    data[0] & 0x01);
    input_report_key(inputdev, BTN_RIGHT,   data[0] & 0x02);
    input_report_key(inputdev, BTN_MIDDLE,  data[0] & 0x04);

    /* Report relative pos */
    input_report_rel(inputdev, REL_X,       data[1]);
    input_report_rel(inputdev, REL_Y,       data[2]);
    input_report_rel(inputdev, REL_WHEEL,   data[3]);

    /* Synchronize */
    input_sync(inputdev);

resubmit:
    status = usb_submit_urb(urb, GFP_ATOMIC);
    if (status) 
        printk("[DRIVER]Cannot resubmit!\n");
}

static int tiny6410_usbmouse_probe(struct usb_interface *intf,
          const struct usb_device_id *id)
{
    struct usb_host_interface *interface;
    struct usb_endpoint_descriptor *epd;
    struct input_dev *usbinput;
    struct tiny6410_usbmouse *usbmouse;
    struct usb_device *usbdev = interface_to_usbdev(intf);
    int err = -ENOMEM;
    int buflen, pipe;

    /* 1. Acquire the current interface settings */
    interface = intf->cur_altsetting;

    /* 
     * Mouse is acting as the only one 'IN' endpoint, 
     * more than one endpoints would be considered as no 
     * such device.
     */
    if (interface->desc.bNumEndpoints != 1)
        return -ENODEV;

    epd = &interface->endpoint[0].desc;
    if (!usb_endpoint_is_int_in(epd))
        return -ENODEV;

    /* 2. Allocating an input_dev and tiny6410_usbmouse structures */
    usbinput = input_allocate_device();
    usbmouse = kzalloc(sizeof(struct tiny6410_usbmouse), GFP_KERNEL);
    if (!usbinput || !usbmouse)
        goto fail1;

    /* 
     * 3. Allocating memory area in order to transfer data 
     *    the data is the pointer points to this area. 
     */
    usbmouse->data = usb_alloc_coherent(usbdev, 8, GFP_ATOMIC, &usbmouse->data_dma);
    if (!usbmouse->data)
        goto fail1;

    /* 4. Allocating URB(USB Request Block) */
    usbmouse->irqurb = usb_alloc_urb(0, GFP_KERNEL);
    if (!usbmouse->irqurb)
        goto fail2;

    usbmouse->usbdev = usbdev;
    usbmouse->inputdev = usbinput;

    /* 5. Setup the input events */
    usbinput->evbit[0] = BIT_MASK(EV_KEY) | BIT_MASK(EV_REL);
    usbinput->keybit[BIT_WORD(BTN_MOUSE)] = BIT_MASK(BTN_LEFT) | BIT_MASK(BTN_RIGHT)
                | BIT_MASK(BTN_MIDDLE);
    usbinput->relbit[0] = BIT_MASK(REL_X) | BIT_MASK(REL_Y) | BIT_MASK(BTN_WHEEL);

    /* 6. Store usbmouse structure into usbinput */
    input_set_drvdata(usbinput, usbmouse);

    /* 7. Initialize usbinput open and close functions */
    //usbinput->open = tiny6410_usbmouse_open;
    //usbinput->close = tiny6410_usbmouse_close;

    
    /* 8. Get the buffer length */
    pipe = usb_rcvintpipe(usbdev, epd->bEndpointAddress);
    buflen = usb_maxpacket(usbdev, pipe, usb_pipeout(pipe));
    
    /* 9. Construct urb struct using irq mode */
    usb_fill_int_urb(usbmouse->irqurb, usbdev, pipe, usbmouse->data, 
                     (buflen > 8 ? 8 : buflen), tiny6410_usbmouse_irq_complete, 
                     usbmouse, epd->bInterval);

    usbmouse->irqurb->transfer_dma = usbmouse->data_dma;
    usbmouse->irqurb->transfer_flags |= URB_NO_TRANSFER_DMA_MAP;

    /* 10. register input device */
    err = input_register_device(usbinput);
    if (err < 0)
        goto fail3;

    /* 11. Store the usbmouse structure to the usb_interface  */
    usb_set_intfdata(intf, usbmouse);

    /* 12. [IMPORTANT]Submit urb to the CORE layer */
    usb_submit_urb(usbmouse->irqurb, GFP_KERNEL);
    
    return 0;

fail3:
    usb_free_urb(usbmouse->irqurb);
fail2:
    usb_free_coherent(usbdev, 8, usbmouse->data, usbmouse->data_dma);
fail1:
    kfree(usbmouse);
    input_free_device(usbinput);
    return err;
}


static void tiny6410_usbmouse_disconnect(struct usb_interface *intf)
{
    struct tiny6410_usbmouse *usbmouse = usb_get_intfdata(intf);

    usb_set_intfdata(intf, NULL);

    if (usbmouse) {
        usb_kill_urb(usbmouse->irqurb);
        input_unregister_device(usbmouse->inputdev);
        usb_free_urb(usbmouse->irqurb);
        usb_free_coherent(usbmouse->usbdev, 8, usbmouse->data, usbmouse->data_dma);
        kfree(usbmouse);
    }
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


