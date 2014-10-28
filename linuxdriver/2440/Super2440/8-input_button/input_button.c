
/* 
 * Referenced by drivers\input\keyboard\gpio_keys.c
 */

#include <linux/module.h>

#include <linux/init.h>
#include <linux/fs.h>
#include <linux/interrupt.h>
#include <linux/irq.h>
#include <linux/sched.h>
#include <linux/pm.h>
#include <linux/slab.h>
#include <linux/sysctl.h>
#include <linux/proc_fs.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/input.h>
#include <linux/gpio_keys.h>
#include <linux/workqueue.h>
#include <linux/gpio.h>

#include <asm/uaccess.h>
#include <asm/io.h>  
#include <asm/irq.h>

#include <mach/regs-gpio.h>
#include <mach/hardware.h>


struct pin_desc {
    int irq;
    char *name;
    unsigned int pin;
	unsigned int key_val;
};


struct pin_desc pins_desc[6] = {
	{ IRQ_EINT0, "k5", S3C2410_GPF(0), KEY_L},  	        // K5
	{ IRQ_EINT1, "K2", S3C2410_GPF(1), KEY_S},  	        // K2
	{ IRQ_EINT2, "K6", S3C2410_GPF(2), KEY_ENTER}, 	        // K6
	{ IRQ_EINT3, "K3", S3C2410_GPF(3), KEY_LEFTSHIFT},  	// K3
	{ IRQ_EINT4, "K7", S3C2410_GPF(4), KEY_RIGHTSHIFT}, 	// K7
	{ IRQ_EINT5, "K4", S3C2410_GPF(5), KEY_Y},  	        // K4	
};


static struct input_dev *input_button_dev;
static struct pin_desc *irq_dev;
static struct timer_list button_timer;

static irqreturn_t buttons_irq(int irq, void *dev_id)
{
    irq_dev = (struct pin_desc *)dev_id;

    /* start the timer after 10ms */
	mod_timer(&button_timer, jiffies+HZ/100);  // HZ = 1s; HZ/100 = 10ms
	return IRQ_RETVAL(IRQ_HANDLED);
}


static void button_timer_handler(unsigned long data)
{   
    struct pin_desc *pindesc = irq_dev;
	unsigned int pinval;

    if(!pindesc)
        return;

	//printk("pindesc->pin = %d\n", pindesc->pin);
	
	pinval = s3c2410_gpio_getpin(pindesc->pin);

	//printk("pinval = %d\n", pinval);
	//printk("pindesc->key_val = %x\n", pindesc->key_val);

	if(pinval) {
		/* key is not pressed down */
		input_event(input_button_dev, EV_KEY, pindesc->key_val, 0);  // 0 means not pressed down, 1 means pressed down
    	input_sync(input_button_dev);
	}else {
		/* key is pressed down */
		input_event(input_button_dev, EV_KEY, pindesc->key_val, 1);  
    	input_sync(input_button_dev);
	}
}


static int input_button_init(void)
{
    int i;
    int error = 0;
    
    /* Allocating an "input_dev" structure */
    input_button_dev = input_allocate_device();
    if (!input_button_dev) {
		error = -ENOMEM;
		goto fail1;
	}
    
    /* Configuring "input_dev" struct */
    set_bit(EV_KEY, input_button_dev->evbit);
    
    set_bit(KEY_L, input_button_dev->keybit);
    set_bit(KEY_S, input_button_dev->keybit);
    set_bit(KEY_ENTER, input_button_dev->keybit);
    set_bit(KEY_LEFTSHIFT, input_button_dev->keybit);
    
    /* Registering "input_dev" */
   	error = input_register_device(input_button_dev);
	if (error) {
        printk("Unable to register input device, error: %d\n", error);
		return error;
	}
    
    /* Hardware related operations */
    init_timer(&button_timer);
    button_timer.function = button_timer_handler;
    add_timer(&button_timer);
    
    for(i = 0; i < 6; ++i) {
        if (request_irq(pins_desc[i].irq, buttons_irq, IRQ_TYPE_EDGE_BOTH, pins_desc[i].name, &pins_desc[i])) {
    		printk(KERN_ERR "input_button.c: Could not allocate button IRQ in request_irq!\n");
		    return -EIO;
	    }
    }
    
fail1:
	input_free_device(input_button_dev);

    
    return error;
}

static void input_button_exit(void)
{
    int i;
    for(i = 0; i < 6; ++i) {
       	free_irq(pins_desc[i].irq, &pins_desc[i]);
    }

    del_timer(&button_timer);
    input_unregister_device(input_button_dev);
    input_free_device(input_button_dev);
}


module_init(input_button_init);
module_exit(input_button_exit);
MODULE_LICENSE("GPL");


