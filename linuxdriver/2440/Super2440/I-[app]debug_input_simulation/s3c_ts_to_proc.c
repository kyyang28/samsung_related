
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
#include <asm/uaccess.h>

#include <plat/regs-adc.h>
#include <mach/regs-gpio.h>

#define AVG_TS(a, b, c, d)          (((a) + (b) + (c) + (d)) / 4)
#define REPLAY_BUF_SIZE             (1024*1024)
#define INPUT_REPLAY                (0)
#define INPUT_TAG                   (1)

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

/* START: Added by Charles.Y on June 3rd, 2012 */
static int major = 0;
static struct class *input_replay_cls;
static struct device *input_replay_dev;

static char *replay_buf;
static int replay_read_pos = 0;
static int replay_write_pos = 0;
static struct timer_list replay_timer;

extern int myprintk(const char *fmt, ...);

static ssize_t replay_write(struct file *file, const char __user *buf, size_t count, loff_t *ppos)
{
    int err;
    /* Store the data from the buf into the replay_buf */
    if (replay_write_pos + count >= REPLAY_BUF_SIZE) {
        printk("s3c_ts[Charles.Y]: replay ring buffer is full!\n");
        return -EIO;
    }
    
    err = copy_from_user(replay_buf + replay_write_pos, buf, count);
    if (err) 
        return -EIO;
    else 
        replay_write_pos += count;

    return count;
}

/* APP: ioctl(fd, CMD, ...) */
static long replay_ioctl(struct file *file, unsigned int cmd, unsigned long arg)
{
    int len;
    char buf[100];
    
    switch (cmd) {
        case INPUT_REPLAY:
            /* 
             *  Start the replay simulation process 
             *  According to the data of the replay_buf to do the input_report
             */
            replay_timer.expires = jiffies + 1;
            add_timer(&replay_timer);            
            break;

        case INPUT_TAG:
            len = copy_from_user(buf, (const void __user *)arg, 100);
            buf[99] = '\0';
            myprintk("%s\n", buf);
            break;
    }
    return 0;
}

static int replayGetLine(char *line)
{
    int i = 0;
    
    /* Eat the space, tab, return symbols */
    while (replay_read_pos <= replay_write_pos) {
        if (replay_buf[replay_read_pos] == ' ' || replay_buf[replay_read_pos] == '\n'
            || replay_buf[replay_read_pos] == '\r' || replay_buf[replay_read_pos] == '\t')
            replay_read_pos++;
        else
            break;
    }

    while (replay_read_pos <= replay_write_pos) {        
        if (replay_buf[replay_read_pos] == '\n' || replay_buf[replay_read_pos] == '\r')
            break;
        else {
            if (i < 100) {
                line[i] = replay_buf[replay_read_pos];
                replay_read_pos++;
                i++;
            } else {
                break;
            }
        }           
    }

    line[i] = '\0';
    return i;
}

static void inputReplayTimerFunction(unsigned long data)
{
    /* 
     *  Acquire the data from the replay_buf and do input_report() 
     *  Read the 1st line, get the timer value, and input_report() the first line
     *  Continue reading the next line, get the data. 
     *      IF the timer is equal to the first timer, 
     *      then 
     *          do input_report()
     *      ELSE
     *          do mod_timer();
     *      
     */
    unsigned int time = 0;
    //unsigned int next_time;
    unsigned int type = 0;
    unsigned int code = 0;
    char line[100];
    int val = 0;
    int ret;

    static unsigned int pre_time = 0, pre_type = 0, pre_code = 0;
    static int pre_val = 0;

    //static int cnt = 0;
    //printk("inputReplayTimerFunction: %d\n", cnt++);

    if (pre_time != 0)
        input_event(s3c_ts_dev, pre_type, pre_code, pre_val);        
    
    while (1) {
        ret = replayGetLine(line);
        if (!ret) { /* No data to acquire */
            printk("End of Input Replay\n");
            del_timer(&replay_timer);
            pre_time = pre_type = pre_code = 0;
            pre_val = 0;
            replay_read_pos = replay_write_pos = 0;
            break;
        }

        /* Processing data */
        sscanf(line, "%x %x %x %d", &time, &type, &code, &val);
        
        //printk("%x %x %x %d\n", time, type, code, val);
        
        if (!time && !type && !code && !val)
            continue;
        else {
            if ((0 == pre_time) || (time == pre_time)) {
                input_event(s3c_ts_dev, type, code, val);

                if (0 == pre_time)
                    pre_time = time;
            } else {
                /* According to the next input_report time, do mod_timer() */
                mod_timer(&replay_timer, jiffies + (time - pre_time));

                pre_time = time;
                pre_type = type;
                pre_code = code;
                pre_val  = val;

                break;
            }
        }        
    }    
}


static struct file_operations replay_fops = {
    .owner          = THIS_MODULE,
    .write          = replay_write,
    .unlocked_ioctl = replay_ioctl,
};
/* END: Added by Charles.Y on June 3rd, 2012 */

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

/* 
 *  Debug purposes: Store the input_event information to /proc/mymsg 
 *  Added by Charles.Y[drivers] on June 2nd, 2012
 *  [START]
 */
static void printOutInputInfo2File(unsigned int time, unsigned int type, unsigned int code, int val)
{
    myprintk("0x%08x 0x%08x 0x%08x %d\n", time, type, code, val);
}
/* [END] June 2nd, 2012 */

static irqreturn_t stylus_updown(int irq, void *dev_id)
{
    if(s3c_ts_regs->ADCDAT0 & (1<<15)) {
        //printk("Stylus up\n");
        input_report_key(s3c_ts_dev, BTN_TOUCH, 0);
        printOutInputInfo2File(jiffies, EV_KEY, BTN_TOUCH, 0); /* 0 means pressed up */
        
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
        printOutInputInfo2File(jiffies, EV_ABS, ABS_PRESSURE, 0);

		input_sync(s3c_ts_dev);
        printOutInputInfo2File(jiffies, EV_SYN, SYN_REPORT, 0);

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
        printOutInputInfo2File(jiffies, EV_KEY, BTN_TOUCH, 0); /* 0 means pressed up */
        
		input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
        printOutInputInfo2File(jiffies, EV_ABS, ABS_PRESSURE, 0);

		input_sync(s3c_ts_dev);
        printOutInputInfo2File(jiffies, EV_SYN, SYN_REPORT, 0);

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
                printOutInputInfo2File(jiffies, EV_ABS, ABS_X, AVG_TS(x[0],x[1],x[2],x[3]));

                input_report_abs(s3c_ts_dev, ABS_Y, AVG_TS(y[0],y[1],y[2],y[3]));
                printOutInputInfo2File(jiffies, EV_ABS, ABS_Y, AVG_TS(y[0],y[1],y[2],y[3]));

                input_report_key(s3c_ts_dev, BTN_TOUCH, 1);
                printOutInputInfo2File(jiffies, EV_KEY, BTN_TOUCH, 1); /* 1 means pressed down */

     			input_report_abs(s3c_ts_dev, ABS_PRESSURE, 1);
                printOutInputInfo2File(jiffies, EV_ABS, ABS_PRESSURE, 1);

 	    		input_sync(s3c_ts_dev);
                printOutInputInfo2File(jiffies, EV_SYN, SYN_REPORT, 0);
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
        /* 
         *  Stylus is up: input_report and print out to /proc/mymsg virtual fs 
         *  print out: jiffies, type, code, value
         */
        input_report_abs(s3c_ts_dev, ABS_PRESSURE, 0);
        printOutInputInfo2File(jiffies, EV_ABS, ABS_PRESSURE, 0);
            
        input_report_key(s3c_ts_dev, BTN_TOUCH, 0);        
        printOutInputInfo2File(jiffies, EV_KEY, BTN_TOUCH, 0); /* 0 means pressed up */

		input_sync(s3c_ts_dev);
        printOutInputInfo2File(jiffies, EV_SYN, SYN_REPORT, 0);

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

    replay_buf = kmalloc(REPLAY_BUF_SIZE, GFP_KERNEL);
    if (!replay_buf) {
        printk("Charles.Y[drivers]: Unable to allocate mylog_buf\n");
        return -EIO;
    }
        
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

    /* START: Added by Charles.Y on June 3rd, 2012 */
    major = register_chrdev(0, "input_replay", &replay_fops);
    
    input_replay_cls = class_create(THIS_MODULE, "input_replay_class");
	if(IS_ERR(input_replay_cls))
		return PTR_ERR(input_replay_cls);

    input_replay_dev = device_create(input_replay_cls, NULL, MKDEV(major, 0), NULL, "inputReplay");
	if(IS_ERR(input_replay_dev))
		return PTR_ERR(input_replay_dev);   

    init_timer(&replay_timer);
    replay_timer.function = inputReplayTimerFunction;
    //add_timer(&replay_timer);
    /* END: Added by Charles.Y on June 3rd, 2012 */
    
    return 0;
}

static void s3c_ts_exit(void)
{
    /* START: Added by Charles.Y on June 3rd, 2012 */
    //del_timer(&replay_timer);
    device_destroy(input_replay_cls, MKDEV(major, 0));
    class_destroy(input_replay_cls);
    unregister_chrdev(major, "input_replay");
    /* END: Added by Charles.Y on June 3rd, 2012 */
    
    del_timer(&ts_timer);
    free_irq(IRQ_TC, NULL);
    free_irq(IRQ_ADC, NULL);
    iounmap(s3c_ts_regs);
    input_unregister_device(s3c_ts_dev);
    input_free_device(s3c_ts_dev);
    
    /* START: Added by Charles.Y on June 3rd, 2012 */
    kfree(replay_buf);
    /* END: Added by Charles.Y on June 3rd, 2012 */
}

module_init(s3c_ts_init);
module_exit(s3c_ts_exit);
MODULE_LICENSE("Dual BSD/GPL");


