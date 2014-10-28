

#include <linux/module.h>		/* For module specific items */
#include <linux/moduleparam.h>		/* For new moduleparam's */
#include <linux/types.h>		/* For standard types (like size_t) */
#include <linux/errno.h>		/* For the -ENODEV/... values */
#include <linux/kernel.h>		/* For printk/panic/... */
#include <linux/fs.h>			/* For file operations */
#include <linux/ioport.h>		/* For io-port access */
#include <linux/platform_device.h>	/* For platform_driver framework */
#include <linux/init.h>			/* For __init/__exit/... */
#include <linux/uaccess.h>		/* For copy_to_user/put_user/... */
#include <linux/io.h>			/* For inb/outb/... */
#include <linux/irq.h>
#include <linux/interrupt.h>
#include <linux/poll.h>
#include <linux/delay.h>
#include <linux/input.h>
#include <linux/clk.h>

struct ts_regs {
	unsigned long adccon;
	unsigned long adctsc;
	unsigned long adcdly;
	unsigned long adcdat0;
	unsigned long adcdat1;
	unsigned long adcupdn;
	unsigned long adcclrint;
	unsigned long reserved;
	unsigned long adcclrintpndnup;
};

static struct input_dev *ts_dev;
static struct ts_regs *ts_regs;
static int x, y;
static int cnt = 0;

static DEFINE_TIMER(ts_timer, NULL, 0, 0);

static void wait_for_pen_down(void)
{
	ts_regs->adctsc = 0xd3;  /* 1101,0011 */
//	ADCTSC  &= ~(1<<8); /* Detect Stylus Down Interrupt Signal. */	
}

static void wait_for_pen_up(void)
{
	ts_regs->adctsc = 0x1d3;
//	ADCTSC  |= (1<<8); /* Detect Stylus Up Interrupt Signal. */
}

static void enter_to_measure_xy(void)
{
	/* (AUTO_PST=1, XY_PST: 2b00 */
	ts_regs->adctsc = (1<<7)|(1<<6)|(1<<4)|(1<<3)|(1<<2);
}

static void enter_to_measure_x(void)
{
	/* (AUTO_PST=0, XY_PST: 2b01 */
	ts_regs->adctsc = 1;
}
static void start_adc(void)
{
	ts_regs->adccon |= (1<<0);
}
static irqreturn_t pen_down_up_isr(int irq, void *dev_ids)
{
	unsigned long data0, data1;
	int down;

	data0 = ts_regs->adcdat0;
	data1 = ts_regs->adcdat1;

	down = (!(data0 & (1<<15))) && (!(data1 & (1<<15)));

	if (!down)
	{
		printk("pen_down_up_isr: up\n");
		wait_for_pen_down();
	}
	else
	{
		printk("pen_down_up_isr: down\n");
		//wait_for_pen_up();
		enter_to_measure_xy();
		//enter_to_measure_x();
		start_adc();
	}

	/* clear interrupt */
	//ts_regs->adcupdn   = 0 ;
	ts_regs->adcclrintpndnup = 0;
	ts_regs->adcclrint = 0 ;
	return IRQ_HANDLED;
}

static irqreturn_t adc_isr(int irq, void *dev_id)
{
	unsigned long data0, data1;
	int down;

#if 0 /* in auto xy mode, after adc interrupt, have to wait several ms to test up/down */ 
	udelay(1000);
	udelay(1000);
	udelay(1000);
	udelay(1000);
#endif
	data0 = ts_regs->adcdat0;
	data1 = ts_regs->adcdat1;
	
	cnt++;
	x += data0 & 0xfff;
	y += data1 & 0xfff;
	if (cnt == 4)
	{
		//printk("x = %05d, y = %05d\n", x/4, y/4);
		mod_timer(&ts_timer, jiffies+1);		
		wait_for_pen_up();
	}
	else
	{
		enter_to_measure_xy();
		//enter_to_measure_x();
		start_adc();
	}

	ts_regs->adcclrintpndnup = 0;
	ts_regs->adcclrint = 0;

	//printk("adc_isr after clear int, data0 = 0x%x, data1 = 0x%x\n", ts_regs->adcdat0, ts_regs->adcdat1);

	return IRQ_HANDLED;
}

void s3c_ts_hardware_setup(void)
{
	struct clk *ts_clk = clk_get(NULL, "adc");
	clk_enable(ts_clk);

	/* clock */
	ts_regs->adccon &= ~((0xff << 6) | (1<<2));

	/* 12-bit A/D conversion, bit16
      * A/D converter prescaler enable, bit14 
      * A/D converter prescaler value = 13, ADC CLK = 66.5MHz/(255+1)=4.75MHz, bit[13:6]
      */
	ts_regs->adccon |= (1<<16) | (1<<14) | (255<<6);  

	ts_regs->adcdly = 0xffff;

	request_irq(IRQ_TC, pen_down_up_isr, 0, "ts_tc", 1);
	request_irq(IRQ_ADC, adc_isr, 0, "ts_adc", 1);
	wait_for_pen_down();
}

static void ts_timer_function(unsigned long data)
{
	unsigned long data0, data1;
	int down;

	data0 = ts_regs->adcdat0;
	data1 = ts_regs->adcdat1;

	down = (!(data0 & (1<<15))) && (!(data1 & (1<<15)));

	if (down)
	{
		if (cnt == 4)
		{
			// report
			input_report_abs(ts_dev, ABS_X, x);
			input_report_abs(ts_dev, ABS_Y, y);

			input_report_key(ts_dev, BTN_TOUCH, 1);
			input_report_abs(ts_dev, ABS_PRESSURE, 1);
			input_sync(ts_dev);

			x = y = cnt = 0;
			mod_timer(&ts_timer, jiffies + HZ * 10 / 1000);
		}
		else
		{
			enter_to_measure_xy();
			start_adc();
		}

	}
	else
	{
		input_report_key(ts_dev, BTN_TOUCH, 0);
		input_report_abs(ts_dev, ABS_PRESSURE, 0);
		input_sync(ts_dev);
		
		x = y = cnt = 0;
		wait_for_pen_down();
	}
}

static int ts_init(void)
{
	/* 1. alloc input_dev */
	ts_dev = input_allocate_device();

	/* 2. setup */
	/* 2.1 which type events ? */
	ts_dev->evbit[0]		= BIT_MASK(EV_ABS) | BIT_MASK(EV_KEY);
	/* 2.2 in these types, which events ? */
	set_bit(BTN_TOUCH, ts_dev->keybit);

	input_set_abs_params(ts_dev, ABS_X, 0, 0xFFF, 0, 0);
	input_set_abs_params(ts_dev, ABS_Y, 0, 0xFFF, 0, 0);
	input_set_abs_params(ts_dev, ABS_PRESSURE, 0, 1, 0, 0);
	
	/* 3. register */
	input_register_device(ts_dev);

	/* 4. hardware setup */
	ts_timer.expires = 0;
	ts_timer.function = ts_timer_function;

	add_timer(&ts_timer);  /* jiffies > expires */

	ts_regs = ioremap(0x7E00B000, 1024);
	s3c_ts_hardware_setup();
	
	return 0;
}

static void ts_exit(void)
{
	del_timer(&ts_timer);
	iounmap(ts_regs);
	free_irq(IRQ_TC, 1);
	free_irq(IRQ_ADC, 1);
	input_unregister_device(ts_dev);
	input_free_device(ts_dev);
}

module_init(ts_init);
module_exit(ts_exit);

MODULE_LICENSE("GPL");

