
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>
#include <linux/cdev.h>
#include <linux/device.h>       /* class_create / class_destroy */
#include <linux/irqreturn.h>    /* irqreturn_t */
#include <linux/interrupt.h>    /* request_irq / free_irq */
#include <asm/io.h>             /* ioremap / iounmap */
#include <asm/uaccess.h>
#include <plat/irqs.h>
#include <mach/irqs.h>

MODULE_LICENSE("GPL");

struct tiny210_buttonsIRQ_desc {
    int number;
    int irqNum;
    int pinNum;
    char *name;
};

static struct tiny210_buttonsIRQ_desc tiny210_buttonsIRQ_desc_arr[] = {
    { 0, IRQ_EINT(16), 0, "Key1"},
    { 1, IRQ_EINT(17), 1, "Key2"},
    { 2, IRQ_EINT(18), 2, "Key3"},
    { 3, IRQ_EINT(19), 3, "Key4"},
    { 4, IRQ_EINT(24), 0, "Key5"},
    { 5, IRQ_EINT(25), 1, "Key6"},
    { 6, IRQ_EINT(26), 2, "Key7"},
    { 7, IRQ_EINT(27), 3, "Key8"},
};

static dev_t tiny210_buttonsIRQ_dev;
static dev_t tiny210_leds_dev;
static struct cdev *tiny210_buttonsIRQ_cdev;
static struct cdev *tiny210_leds_cdev;
static struct class *tiny210_buttonsIRQ_cls;
static struct class *tiny210_leds_cls;

/* the GPIO of the buttonsIRQ */
static volatile unsigned long *GPH2DAT = NULL;
static volatile unsigned long *GPH3DAT = NULL;

/* the GPIO of the leds */
static volatile unsigned long *GPJ2CON = NULL;
static volatile unsigned long *GPJ2DAT = NULL;

static DECLARE_WAIT_QUEUE_HEAD(tiny210_buttonsIRQ_wq);
static int condition = 0;

static int buttons_keyval[8] = { 0, 0, 0, 0, 0, 0, 0, 0 };

static irqreturn_t tiny210_buttonsIRQ_handler(int irq, void *dev_id)
{
    struct tiny210_buttonsIRQ_desc *buttonsIRQ_tmp = (struct tiny210_buttonsIRQ_desc *)dev_id;
    int isPressed;
    
    if ((irq == IRQ_EINT(16)) || (irq == IRQ_EINT(17)) || (irq == IRQ_EINT(18)) || (irq == IRQ_EINT(19))) {
        isPressed = !(*GPH2DAT & (1<<buttonsIRQ_tmp->pinNum));
        buttons_keyval[buttonsIRQ_tmp->number] = isPressed;
    }else if ((irq == IRQ_EINT(24)) || (irq == IRQ_EINT(25)) || (irq == IRQ_EINT(26)) || (irq == IRQ_EINT(27))) {
        isPressed = !(*GPH3DAT & (1<<buttonsIRQ_tmp->pinNum));
        buttons_keyval[buttonsIRQ_tmp->number] = isPressed;
    }
    
    condition = 1;
    wake_up_interruptible(&tiny210_buttonsIRQ_wq);
    
    return IRQ_HANDLED;
}

static int tiny210_buttonsIRQ_open(struct inode *inode, struct file *filp)
{
    int i;
    
    /* Request buttons IRQ interruptions */
    for (i = 0; i < ARRAY_SIZE(tiny210_buttonsIRQ_desc_arr); i++)
        if (request_irq(tiny210_buttonsIRQ_desc_arr[i].irqNum, tiny210_buttonsIRQ_handler,
                    IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING,
                    tiny210_buttonsIRQ_desc_arr[i].name, &tiny210_buttonsIRQ_desc_arr[i]))
            return -EAGAIN;
    
    return 0;
}

static int tiny210_buttonsIRQ_release(struct inode *inode, struct file *filp)
{
    int i;

    for (i = 0; i < ARRAY_SIZE(tiny210_buttonsIRQ_desc_arr); i++)
        free_irq(tiny210_buttonsIRQ_desc_arr[i].irqNum, &tiny210_buttonsIRQ_desc_arr[i]);
    
    return 0;
}

static ssize_t tiny210_buttonsIRQ_read(struct file *filp, char __user *buf, 
            size_t count, loff_t *ppos)
{
    wait_event_interruptible(tiny210_buttonsIRQ_wq, condition);

    condition = 0;

    if (copy_to_user(buf, buttons_keyval, sizeof(buttons_keyval)))
        return -EFAULT;
    
    return sizeof(buttons_keyval);
}

static int tiny210_leds_open(struct inode *inode, struct file *filp)
{
    *GPJ2CON = 0x1<<12 | 0x1<<8 | 0x1<<4 | 0x1<<0;
    return 0;
}

static long tiny210_leds_ioctl(struct file *filp, unsigned int cmd, unsigned long arg)
{
    switch (arg) {
    case 1:
        if (!cmd)
            *GPJ2DAT &= ~(1<<0);
        else
            *GPJ2DAT |= (1<<0);
        break;

    case 2:
        if (!cmd)
            *GPJ2DAT &= ~(1<<1);
        else
            *GPJ2DAT |= (1<<1);
        break;

    case 3:
        if (!cmd)
            *GPJ2DAT &= ~(1<<2);
        else
            *GPJ2DAT |= (1<<2);
        break;

    case 4:
        if (!cmd)
            *GPJ2DAT &= ~(1<<3);
        else
            *GPJ2DAT |= (1<<3);
        break;

    default:
        return -EINVAL;
    }
    
    return 0;
}

static const struct file_operations tiny210_buttonsIRQ_fops = {
    .owner              = THIS_MODULE,
    .open               = tiny210_buttonsIRQ_open,
    .release            = tiny210_buttonsIRQ_release,
    .read               = tiny210_buttonsIRQ_read,
};

static struct file_operations tiny210_leds_fops = {
    .owner              = THIS_MODULE,
    .open               = tiny210_leds_open,
    .unlocked_ioctl     = tiny210_leds_ioctl,
};

static int __init tiny210_buttonsIRQ_drv_init(void)
{
    int ret;
    
    /* Seq 1: Allocating the Major/Minor */
    ret = alloc_chrdev_region(&tiny210_buttonsIRQ_dev, 0, 1, "tiny210_buttonsIRQ");
	if (ret < 0) {
		printk(KERN_WARNING "tiny210_buttonsIRQ_drv_init: can't get buttonsIRQ major number\n");
		return ret;
	}

    ret = alloc_chrdev_region(&tiny210_leds_dev, 0, 1, "tiny210_leds");
	if (ret < 0) {
		printk(KERN_WARNING "tiny210_buttonsIRQ_drv_init: can't get leds major number\n");
		return ret;
	}
    
    /* Seq 2: Allocating the cdev struct */
    tiny210_buttonsIRQ_cdev = cdev_alloc();
    tiny210_leds_cdev = cdev_alloc();

    /* Seq 3: Initializting the cdev struct */
    cdev_init(tiny210_buttonsIRQ_cdev, &tiny210_buttonsIRQ_fops);
    tiny210_buttonsIRQ_cdev->owner = tiny210_buttonsIRQ_fops.owner;

    cdev_init(tiny210_leds_cdev, &tiny210_leds_fops);
    tiny210_leds_cdev->owner = tiny210_leds_fops.owner;

    /* Seq 4: Adding the cdev struct */
    cdev_add(tiny210_buttonsIRQ_cdev, tiny210_buttonsIRQ_dev, 1);
    cdev_add(tiny210_leds_cdev, tiny210_leds_dev, 1);

    /* Seq 5: Class create / Device create */
    tiny210_buttonsIRQ_cls = class_create(THIS_MODULE, "tiny210_buttonsIRQ");
    tiny210_leds_cls = class_create(THIS_MODULE, "tiny210_leds");
    device_create(tiny210_buttonsIRQ_cls, NULL, tiny210_buttonsIRQ_dev, NULL, "tiny210ButtonsIRQ");
    device_create(tiny210_leds_cls, NULL, tiny210_leds_dev, NULL, "tiny210Leds");
    
    /* Seq 6: Hardware related setup */
    GPH2DAT = (volatile unsigned long *)ioremap(0xE0200C44, 4);
    GPH3DAT = (volatile unsigned long *)ioremap(0xE0200C64, 4);

    GPJ2CON = (volatile unsigned long *)ioremap(0xE0200280, 8);
    GPJ2DAT = GPJ2CON + 1;

    printk("tiny210_buttonsIRQ_leds module is inserted into the kernel\n");
    
    return 0;
}
module_init(tiny210_buttonsIRQ_drv_init);

static void __exit tiny210_buttonsIRQ_drv_exit(void)
{    
    /* Seq 1: Hardware related cleanup */
    iounmap(GPJ2CON);
    iounmap(GPH3DAT);
    iounmap(GPH2DAT);

    /* Seq 2: Class destroy / Device destroy */
    device_destroy(tiny210_leds_cls, tiny210_leds_dev);
    device_destroy(tiny210_buttonsIRQ_cls, tiny210_buttonsIRQ_dev);
    class_destroy(tiny210_leds_cls);
    class_destroy(tiny210_buttonsIRQ_cls);

    /* Seq 3: Deleting the cdev struct */
    cdev_del(tiny210_leds_cdev);
    cdev_del(tiny210_buttonsIRQ_cdev);

    /* Seq 4: Unregistering the cdev */
    unregister_chrdev_region(tiny210_leds_dev, 1);
    unregister_chrdev_region(tiny210_buttonsIRQ_dev, 1);

    printk("tiny210_buttonsIRQ_leds module is removed from the kernel\n");
}
module_exit(tiny210_buttonsIRQ_drv_exit);

