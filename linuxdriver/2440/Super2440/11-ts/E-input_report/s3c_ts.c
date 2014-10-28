
#include <linux/errno.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/slab.h>
#include <linux/input.h>
#include <linux/init.h>
#include <linux/serio.h>
#include <linux/delay.h>
#include <linux/platform_device.h>
#include <linux/clk.h>
#include <linux/gpio.h>
#include <asm/io.h>
#include <asm/irq.h>

#include <plat/regs-adc.h>
#include <mach/regs-gpio.h>

#define AVG_TS(a, b, c, d)         (((a) + (b) + (c) + (d)) / 4)

struct s3c_ts_regs {
    unsigned long ADCCON;
    unsigned long ADCTSC;
    unsigned long ADCDLY;
    unsigned long ADCDAT0;
    unsigned long ADCDAT1;
    unsigned long ADCUPDN;    
};

static struct input_dev *s3c_ts_dev;

static volatile struct s3c_ts_regs *s3c_ts_regs;

static struct timer_list ts_timer;


static void wait4IntMode_Down(void)
{
    s3c_ts_regs->ADCTSC = 0xD3; /* 0xD3 = 0000 1101 0011 */ 
}


static void wait4IntMode_Up(void)
{
    s3c_ts_regs->ADCTSC = 0x1D3;
}


static void measure_xy_mode(void)
{
    s3c_ts_regs->ADCTSC = (1<<3) | (1<<2);
}


static void start_adc(void)
{
    s3c_ts_regs->ADCCON |= (1<<0);
}


static irqreturn_t stylus_updown(int irq, void *dev_id)
{
    if(s3c_ts_regs->ADCDAT0 & (1<<15)) {
        //printk("Stylus up\n");
        input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		input_sync(s3c_ts_dev);
        wait4IntMode_Down();
    }else {
        //printk("Stylus down\n");
        //wait4IntMode_Up();
        measure_xy_mode();
        start_adc();
    }
    
	return IRQ_HANDLED;    
}


static int s3c_filter_ts(long int x[], long int y[])
{
#define ERR_LIMIT 10

    int avg_x, avg_y, delta_x, delta_y;
    
    avg_x = (x[0] + x[1]) / 2;
    avg_y = (y[0] + y[1]) / 2;

    delta_x = (x[2] > avg_x) ? (x[2] - avg_x) : (avg_x - x[2]);
    delta_y = (y[2] > avg_y) ? (y[2] - avg_y) : (avg_y - y[2]);

    if ((delta_x > ERR_LIMIT) || (delta_y > ERR_LIMIT))
        return 0;

    avg_x = (x[1] + x[2]) / 2;
    avg_y = (y[1] + y[2]) / 2;

    delta_x = (x[3] > avg_x) ? (x[3] - avg_x) : (avg_x - x[3]);
    delta_y = (y[3] > avg_y) ? (y[3] - avg_y) : (avg_y - y[3]);

    if ((delta_x > ERR_LIMIT) || (delta_y > ERR_LIMIT))
        return 0;

    return 1;
}


static irqreturn_t stylus_action(int irq, void *dev_id)
{
    static int cnt = 0;
    long int adcdat0, adcdat1;
    static long int x[4], y[4];
    
    /* 
     *  Extra: Optimization 2 
     *  After the ADC interrupt is finished, if the stylus is up, then discards this results
     */
    adcdat0 = s3c_ts_regs->ADCDAT0;
    adcdat1 = s3c_ts_regs->ADCDAT1;

    if(s3c_ts_regs->ADCDAT0 & (1<<15)) {
        /* Stylus is already up */
        cnt = 0;
        input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		input_sync(s3c_ts_dev);
        wait4IntMode_Down();
    }else {
        //printk("stylus_action cnt = %d, (x,y) = (%ld,%ld)\n", ++cnt, adcdat0 & 0x3FF, adcdat1 & 0x3FF);

        /* 
         *  Extra: Optimization 3 
         *  Measure many times and calculate the average value
         */
        x[cnt] = adcdat0 & 0x3FF;
        y[cnt] = adcdat1 & 0x3FF;
        ++cnt;
        if(4 == cnt) {
            /*  
             *  Extra: Optimization 4
             *  Software filter
             */
            if(s3c_filter_ts(x, y)) {
                //printk("(x,y) = (%ld,%ld)\n", AVG_TS(x[0],x[1],x[2],x[3]), AVG_TS(y[0],y[1],y[2],y[3]));  
                input_report_abs(s3c_ts_dev, ABS_X, AVG_TS(x[0],x[1],x[2],x[3]));
                input_report_abs(s3c_ts_dev, ABS_Y, AVG_TS(y[0],y[1],y[2],y[3]));

                input_report_key(s3c_ts_dev, BTN_TOUCH, 1);
     			input_report_abs(s3c_ts_dev, ABS_PRESSURE, 1);
 	    		input_sync(s3c_ts_dev);
            }
            cnt = 0;
            wait4IntMode_Up();

            /* Start timer for handling the slither */
            mod_timer(&ts_timer, jiffies + HZ/100);  /* HZ/100 = 10ms since HZ = 1s */
            
        }else {            
            measure_xy_mode();
            start_adc();
        }
    }

    return IRQ_HANDLED;
}


static void s3c_ts_timer_function(unsigned long data)
{
    if(s3c_ts_regs->ADCDAT0 & (1<<15)) {
        /* Stylus is up */
        input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
		input_sync(s3c_ts_dev);

        wait4IntMode_Down();
    }else {
        /* Measure X/Y axis values */
        measure_xy_mode();
        start_adc();
    }
}


static int s3c_ts_init(void)
{
	struct clk *adc_clock;
    
    /* 1. Allocating an input_dev structure */
    s3c_ts_dev = input_allocate_device();
    
    /* 2. Configure */
    set_bit(EV_KEY, s3c_ts_dev->evbit);
    set_bit(EV_ABS, s3c_ts_dev->evbit);    
    set_bit(BTN_TOUCH, s3c_ts_dev->keybit);

	input_set_abs_params(s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);
    
    /* 3. Register */
    input_register_device(s3c_ts_dev);
    
    /* 4. Hardware related operations */
    /* 4.1 Enable clock - CLKCON register */
    adc_clock = clk_get(NULL, "adc");
    clk_enable(adc_clock);

    /* 4.2 Setup the ADC/TS registers */
    s3c_ts_regs = ioremap(0x58000000, sizeof(struct s3c_ts_regs));

    /*
     *  ADCCON 0x5800000 R/W ADC Control Register
     *
     *  PRSCEN [14]     A/D converter prescaler enable 
     *                  0 = Disable 
     *                  1 = Enable      (Selected)
     *  PRSCVL [13:6]   A/D converter prescaler value 
     *                  Data value: 0 ~ 255 
     *                  NOTE: ADC Freqeuncy should be set less than PCLK by 5times. 
     *                  (Ex. PCLK=10MHZ, ADC Freq.< 2MHz)
     *                  Here, we use PRSCVL = 49
     *                  so, ADCCLK = PCLK / (PRSCVL + 1) = 50MHz / (49 + 1) = 1MHz
     *  STDBM [2]   Standby mode select 
     *              0 = Normal operation mode (Selected) 
     *              1 = Standby mode 
     *  READ_ START [1] A/D conversion start by read 
     *                  0 = Disable start by read operation 
     *                  1 = Enable start by read operation
     *  ENABLE_START [0] A/D conversion starts by enable. 
     *                   If READ_START is enabled, this value is not valid. 
     *                   0 = No operation 
     *                   1 = A/D conversion starts and this bit is cleared after the start-up.
     */    
    s3c_ts_regs->ADCCON = (1<<14) | (49<<6);

    /* Register IRQ */
//	if (request_irq(IRQ_ADC, stylus_action, IRQF_SHARED|IRQF_SAMPLE_RANDOM, "s3c_action", NULL)) { /* not work by adding IRQF_SHARED */
	if (request_irq(IRQ_ADC, stylus_action, IRQF_SAMPLE_RANDOM, "s3c_action", NULL)) {
        printk(KERN_ERR "s3c_ts.c: Could not allocate ts IRQ_ADC !\n");
    	iounmap(s3c_ts_regs);
    	return -EIO;
	}

    if (request_irq(IRQ_TC, stylus_updown, IRQF_SAMPLE_RANDOM, "s3c_action", NULL)) {
		printk(KERN_ERR "s3c_ts.c: Could not allocate ts IRQ_TC !\n");
		iounmap(s3c_ts_regs);
		return -EIO;
	}

    /* 
     *  Extra: Optimization 1 
     *  Setup the ADCDLY register to the max value so that the IRQ_TC interrupt is triggered after
     *  the power is stabilized.
     */
    s3c_ts_regs->ADCDLY = 0xFFFF;

    /* Extra: Optimization 5(Timer) */
    init_timer(&ts_timer);
    ts_timer.function = s3c_ts_timer_function;
    add_timer(&ts_timer);
    
    wait4IntMode_Down();
    
    return 0;
}

static void s3c_ts_exit(void)
{
    del_timer(&ts_timer);
    free_irq(IRQ_TC, NULL);
    free_irq(IRQ_ADC, NULL);
    iounmap(s3c_ts_regs);
    input_unregister_device(s3c_ts_dev);
    input_free_device(s3c_ts_dev);
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("Dual BSD/GPL");


