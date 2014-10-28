

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
#include <mach/map.h>
#include <mach/irqs.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


struct key_desc {
    int irq;
    char *name;
    unsigned int pin;
	unsigned int key_val;
    unsigned int pin_val;
};

struct key_desc keys_desc[] = {
	{ IRQ_EINT8,  "K1",  0, KEY_L},	   
	{ IRQ_EINT11, "K2",  3, KEY_S},	  
	{ IRQ_EINT13, "K3",  5, KEY_ENTER},
	{ IRQ_EINT14, "K4",  6, KEY_P},
	{ IRQ_EINT15, "K5",  7, KEY_W},
	{ IRQ_EINT19, "K6", 11, KEY_D},
};


static struct input_dev *mini2440_input_key;
static struct timer_list keys_timers;
static struct key_desc *irq_dev;


static void keys_timer_handler(unsigned long data)
{
    unsigned int pinval;
    unsigned int tmp;

    if (!irq_dev)
        return;

    tmp = readl(S3C2410_GPGDAT);
    pinval = tmp & (1<<irq_dev->pin);
    if (pinval != irq_dev->pin_val)
        return;
    
    if (pinval) {
        /* Key is not pressed down */
        input_event(mini2440_input_key, EV_KEY, irq_dev->key_val, 0); // 0 means not pressed down, 1 means pressed down
        input_sync(mini2440_input_key);
    }else {
        /* Key is pressed down */
        input_event(mini2440_input_key, EV_KEY, irq_dev->key_val, 1);
        input_sync(mini2440_input_key);
    }
}

static irqreturn_t mini2440_keys_irq(int irq, void *dev_id)
{
    unsigned int tmp;
    irq_dev = (struct key_desc *)dev_id;

    tmp = readl(S3C2410_GPGDAT);
    irq_dev->pin_val = tmp & (1<<irq_dev->pin);
    
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
        if (request_irq(keys_desc[i].irq, mini2440_keys_irq, IRQ_TYPE_EDGE_BOTH, keys_desc[i].name, &keys_desc[i])) {
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

static int __init mini2440_input_key_init(void)
{
    int err;
    
    /* Allocating input device */
    mini2440_input_key = input_allocate_device();
    if (!mini2440_input_key)
        return -ENOMEM;

    /* Setup input_dev(mini2440_input_key) structure */
    /* Key event */
    set_bit(EV_KEY,         mini2440_input_key->evbit);
    /* Repeat event */
    set_bit(EV_REP,         mini2440_input_key->evbit);
    
    set_bit(KEY_D,          mini2440_input_key->keybit);
    set_bit(KEY_L,          mini2440_input_key->keybit);
    set_bit(KEY_P,          mini2440_input_key->keybit);
    set_bit(KEY_S,          mini2440_input_key->keybit);
    set_bit(KEY_W,          mini2440_input_key->keybit);
    set_bit(KEY_ENTER,      mini2440_input_key->keybit);
    set_bit(KEY_LEFTSHIFT,  mini2440_input_key->keybit);
    set_bit(KEY_RIGHTSHIFT, mini2440_input_key->keybit);

    /* Register input_dev(mini2440_input_key) structure */
    err = input_register_device(mini2440_input_key);
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

static void __exit mini2440_input_key_exit(void)
{
    del_timer(&keys_timers);
    free_keys_irq();
    input_unregister_device(mini2440_input_key);
    input_free_device(mini2440_input_key);
}

module_init(mini2440_input_key_init);
module_exit(mini2440_input_key_exit);

