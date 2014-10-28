

#include <linux/module.h>  
#include <linux/kernel.h>
#include <linux/init.h> 
#include <linux/fs.h>   
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/device.h>
#include <linux/cdev.h>
#include <linux/slab.h>             /* kmalloc/kfree */
#include <linux/interrupt.h>        /* request_irq */
#include <linux/irq.h>              /* IRQ_TYPE_EDGE_BOTH */
#include <linux/sched.h>            /* TASK_INTERRUPTIBLE */
#include <linux/miscdevice.h>       /* miscdevice, etc */
#include <linux/input.h>            /* input subsystem */
#include <linux/gpio.h>
#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>
#include <mach/regs-gpio.h>
#include <mach/hardware.h>
#include <mach/gpio-bank-n.h>
#include <mach/gpio-bank-l.h>
#include <mach/map.h>
#include <mach/irqs.h>


struct key_desc {
    int irq;
    char *name;
    unsigned int pin;
	unsigned int key_val;
    unsigned int pin_val;
};

struct key_desc keys_desc[] = {
	{ IRQ_EINT( 0), "KEY1",  0, KEY_L},	   
	{ IRQ_EINT( 1), "KEY2",  1, KEY_S},	  
	{ IRQ_EINT( 2), "KEY3",  2, KEY_ENTER},
	{ IRQ_EINT( 3), "KEY4",  3, KEY_D},
	{ IRQ_EINT( 4), "KEY5",  4, KEY_P},
	{ IRQ_EINT( 5), "KEY6",  5, KEY_W},
    { IRQ_EINT(19), "KEY7", 11, KEY_LEFTSHIFT},
    { IRQ_EINT(20), "KEY8", 12, KEY_RIGHTSHIFT},
};


static struct input_dev *tiny6410_input_key;
static struct timer_list keys_timers;
static struct key_desc *irq_dev;


static void keys_timer_handler(unsigned long data)
{
    unsigned int pinval;
    unsigned int tmp;

    if (!irq_dev)
        return;

    if ((irq_dev->irq == IRQ_EINT(19)) || (irq_dev->irq == IRQ_EINT(20))) {
        tmp = readl(S3C64XX_GPLDAT);
        pinval = tmp & (1<<irq_dev->pin);
        if (pinval != irq_dev->pin_val)
            return;
        
        if (pinval) {
            /* Key is not pressed down */
            input_event(tiny6410_input_key, EV_KEY, irq_dev->key_val, 0); // 0 means not pressed down, 1 means pressed down
            input_sync(tiny6410_input_key);
        }else {
            /* Key is pressed down */
            input_event(tiny6410_input_key, EV_KEY, irq_dev->key_val, 1);
            input_sync(tiny6410_input_key);
        }
    }else {
        tmp = readl(S3C64XX_GPNDAT);
        pinval = tmp & (1<<irq_dev->pin);
        if (pinval != irq_dev->pin_val)
            return;
        
        if (pinval) {
            /* Key is not pressed down */
            input_event(tiny6410_input_key, EV_KEY, irq_dev->key_val, 0); // 0 means not pressed down, 1 means pressed down
            input_sync(tiny6410_input_key);
        }else {
            /* Key is pressed down */
            input_event(tiny6410_input_key, EV_KEY, irq_dev->key_val, 1);
            input_sync(tiny6410_input_key);
        }
    }
}

static irqreturn_t tiny6410_keys_irq(int irq, void *dev_id)
{
    unsigned int tmp;
    irq_dev = (struct key_desc *)dev_id;

    if ((irq_dev->irq == IRQ_EINT(19)) || (irq_dev->irq == IRQ_EINT(20))) {
        tmp = readl(S3C64XX_GPLDAT);
        irq_dev->pin_val = tmp & (1<<irq_dev->pin);
    }else {
        tmp = readl(S3C64XX_GPNDAT);
        irq_dev->pin_val = tmp & (1<<irq_dev->pin);
    }
    
    /* Start the timer after 10ms */
    mod_timer(&keys_timers, jiffies+HZ/100);
    return IRQ_RETVAL(IRQ_HANDLED);
}

static void setup_keys_timer(void)
{
    init_timer(&keys_timers);
    keys_timers.function = keys_timer_handler;
    add_timer(&keys_timers);
}

static int request_keys_irq(void)
{  
    int i;
    for(i = 0; i < (sizeof(keys_desc)/sizeof(keys_desc[0])); ++i) {
        if (request_irq(keys_desc[i].irq, tiny6410_keys_irq, IRQ_TYPE_EDGE_BOTH, keys_desc[i].name, &keys_desc[i])) {
    		printk(KERN_ERR "input_button.c: Could not allocate key IRQ in request_irq!\n");
		    return -EIO;
	    }
    }
    return 0;
}

static void free_keys_irq(void)
{
    int i;
    for(i = 0; i < (sizeof(keys_desc)/sizeof(keys_desc[0])); ++i) {
       	free_irq(keys_desc[i].irq, &keys_desc[i]);
    }
}

static int __init tiny6410_input_key_init(void)
{
    int err;
    
    /* Allocating input device */
    tiny6410_input_key = input_allocate_device();
    if (!tiny6410_input_key)
        return -ENOMEM;

    /* Setup input_dev(tiny6410_input_key) structure */
    /* Key event */
    set_bit(EV_KEY,         tiny6410_input_key->evbit);
    /* Repeat event */
    set_bit(EV_REP,         tiny6410_input_key->evbit);
    
    set_bit(KEY_D,          tiny6410_input_key->keybit);
    set_bit(KEY_L,          tiny6410_input_key->keybit);
    set_bit(KEY_P,          tiny6410_input_key->keybit);
    set_bit(KEY_S,          tiny6410_input_key->keybit);
    set_bit(KEY_W,          tiny6410_input_key->keybit);
    set_bit(KEY_ENTER,      tiny6410_input_key->keybit);
    set_bit(KEY_LEFTSHIFT,  tiny6410_input_key->keybit);
    set_bit(KEY_RIGHTSHIFT, tiny6410_input_key->keybit);

    /* Register input_dev(tiny6410_input_key) structure */
    err = input_register_device(tiny6410_input_key);
    if (err) {
        printk("Unable to register input device, error: %d\n", err);
        return err;
    }

    /* request_irq for keys */
    err = request_keys_irq();
    if (err < 0)
        return err;

    /* Timer operations */    
    setup_keys_timer();
    
    return 0;
}

static void __exit tiny6410_input_key_exit(void)
{
    del_timer(&keys_timers);
    free_keys_irq();
    input_unregister_device(tiny6410_input_key);
    input_free_device(tiny6410_input_key);
}

module_init(tiny6410_input_key_init);
module_exit(tiny6410_input_key_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Charles Yang <charlesyang28@gmail.com>");

