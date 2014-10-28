

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
#include <asm/io.h>
#include <asm/irq.h>

#include <plat/regs-adc.h>


MODULE_LICENSE("GPL");
MODULE_AUTHOR("Pete&Charles <http://embeddedfansclub.taobao.com>");


struct mini2440_ts {
    struct input_dev *s3c_ts_dev;
    struct timer_list ts_timer;
	struct clk* clk;
    struct resource *ts_mem;
    struct resource *res_mem;
    struct resource *res_irq_tc;
    struct resource *res_irq_adc;            
    void __iomem *ts_virt;
};

static struct mini2440_ts ts;

static void enter_wait_pen_down_mode(void)
{
    writel(0xd3, ts.ts_virt + S3C2410_ADCTSC);
}

static void enter_wait_pen_up_mode(void)
{
    writel(0x1d3, ts.ts_virt + S3C2410_ADCTSC);
}

static void enter_measure_xy_mode(void)
{
    writel(((1<<3)|(1<<2)), ts.ts_virt + S3C2410_ADCTSC);
}

static void start_adc(void)
{
    writel((readl(ts.ts_virt + S3C2410_ADCCON) | (1<<0)), ts.ts_virt + S3C2410_ADCCON);
}

static int s3c_filter_ts(int x[], int y[])
{
#define ERR_LIMIT 10

	int avr_x, avr_y;
	int det_x, det_y;

	avr_x = (x[0] + x[1])/2;
	avr_y = (y[0] + y[1])/2;

	det_x = (x[2] > avr_x) ? (x[2] - avr_x) : (avr_x - x[2]);
	det_y = (y[2] > avr_y) ? (y[2] - avr_y) : (avr_y - y[2]);

	if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;

	avr_x = (x[1] + x[2])/2;
	avr_y = (y[1] + y[2])/2;

	det_x = (x[3] > avr_x) ? (x[3] - avr_x) : (avr_x - x[3]);
	det_y = (y[3] > avr_y) ? (y[3] - avr_y) : (avr_y - y[3]);

	if ((det_x > ERR_LIMIT) || (det_y > ERR_LIMIT))
		return 0;
	
	return 1;
}

static void s3c_ts_timer_function(unsigned long data)
{
    unsigned long tmp = readl(ts.ts_virt + S3C2410_ADCDAT0);

    if (tmp & (1<<15)) {
		/* �Ѿ��ɿ� */
		input_report_abs(ts.s3c_ts_dev, ABS_PRESSURE, 0);
		input_report_key(ts.s3c_ts_dev, BTN_TOUCH, 0);
		input_sync(ts.s3c_ts_dev);
		enter_wait_pen_down_mode();
	}else {
		/* ����X/Y���� */
		enter_measure_xy_mode();
		start_adc();
	}
}

static irqreturn_t pen_down_up_irq(int irq, void *dev_id)
{
    unsigned long tmp = readl(ts.ts_virt + S3C2410_ADCDAT0);

	if (tmp & (1<<15)) {
		input_report_abs(ts.s3c_ts_dev, ABS_PRESSURE, 0);
		input_report_key(ts.s3c_ts_dev, BTN_TOUCH, 0);
		input_sync(ts.s3c_ts_dev);
		enter_wait_pen_down_mode();
	}else {
		//enter_wait_pen_up_mode();
		enter_measure_xy_mode();
		start_adc();
	}
    
	return IRQ_HANDLED;
}

static irqreturn_t adc_irq(int irq, void *dev_id)
{
	static int cnt = 0;
	static int x[4], y[4];
	int adcdat0, adcdat1;
	
	
	/* �Ż���ʩ2: ���ADC���ʱ, ���ִ������Ѿ��ɿ�, �����˴ν�� */
	//adcdat0 = s3c_ts_regs->adcdat0;
	//adcdat1 = s3c_ts_regs->adcdat1;
	adcdat0 = readl(ts.ts_virt + S3C2410_ADCDAT0);  //s3c_ts_regs->adcdat0;
	adcdat1 = readl(ts.ts_virt + S3C2410_ADCDAT1);  //s3c_ts_regs->adcdat1;

	if (adcdat0 & (1<<15)) {
		/* �Ѿ��ɿ� */
		cnt = 0;
		input_report_abs(ts.s3c_ts_dev, ABS_PRESSURE, 0);
		input_report_key(ts.s3c_ts_dev, BTN_TOUCH, 0);
		input_sync(ts.s3c_ts_dev);
		enter_wait_pen_down_mode();
	}else {
		//printk("adc_irq cnt = %d, x = %d, y = %d\n", ++cnt, adcdat0 & 0x3ff, adcdat1 & 0x3ff);
		/* �Ż���ʩ3: ��β�����ƽ��ֵ */
		x[cnt] = adcdat0 & 0x3ff;
		y[cnt] = adcdat1 & 0x3ff;
		++cnt;
		if (cnt == 4) {
			/* �Ż���ʩ4: ������� */
			if (s3c_filter_ts(x, y)) {			
				//printk("x = %d, y = %d\n", (x[0]+x[1]+x[2]+x[3])/4, (y[0]+y[1]+y[2]+y[3])/4);
				input_report_abs(ts.s3c_ts_dev, ABS_X, (x[0]+x[1]+x[2]+x[3])/4);
				input_report_abs(ts.s3c_ts_dev, ABS_Y, (y[0]+y[1]+y[2]+y[3])/4);
				input_report_abs(ts.s3c_ts_dev, ABS_PRESSURE, 1);
				input_report_key(ts.s3c_ts_dev, BTN_TOUCH, 1);
				input_sync(ts.s3c_ts_dev);
			}
			cnt = 0;
			enter_wait_pen_up_mode();

			/* ������ʱ��������/��������� */
			mod_timer(&ts.ts_timer, jiffies + HZ/100);
		}else {
			enter_measure_xy_mode();
			start_adc();
		}		
	}
	
	return IRQ_HANDLED;
}


static int mini2440_ts_drv_probe(struct platform_device *pdev)
{
    int retval;
    int size;
    unsigned long tmp;

	/* 1. ����һ��input_dev�ṹ�� */
	ts.s3c_ts_dev = input_allocate_device();
    if (!ts.s3c_ts_dev) {
        retval = -ENOMEM;
        goto input_alloc_fail;
    }
    
	/* 2. ���� */
	/* 2.1 �ܲ��������¼� */
	set_bit(EV_KEY, ts.s3c_ts_dev->evbit);
	set_bit(EV_ABS, ts.s3c_ts_dev->evbit);

	/* 2.2 �ܲ��������¼������Щ�¼� */
	set_bit(BTN_TOUCH, ts.s3c_ts_dev->keybit);

	input_set_abs_params(ts.s3c_ts_dev, ABS_X, 0, 0x3FF, 0, 0);
	input_set_abs_params(ts.s3c_ts_dev, ABS_Y, 0, 0x3FF, 0, 0);
	input_set_abs_params(ts.s3c_ts_dev, ABS_PRESSURE, 0, 1, 0, 0);
    
	/* 3. ע�� */
	retval = input_register_device(ts.s3c_ts_dev);
    if (retval < 0)
        goto dealloc_input_dev;

	/* 4. Ӳ����صĲ��� */
	/* 4.1 ʹ��ʱ��(CLKCON[15]) */
	ts.clk = clk_get(NULL, "adc");
	clk_enable(ts.clk);
    
    ts.res_mem = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (NULL == ts.res_mem) {
        dev_err(&pdev->dev, "failed to get memory registers\n");
        retval = -ENXIO;
        goto fail_get_res;
    }

	size = resource_size(ts.res_mem);
    ts.ts_mem = request_mem_region(ts.res_mem->start, size, pdev->name);
	if (NULL == ts.ts_mem) {
		dev_err(&pdev->dev, "failed to get memory region\n");
		retval = -ENOENT;
		goto fail_get_res;
	}

    ts.ts_virt = ioremap(ts.res_mem->start, size);    
	if (NULL == ts.ts_virt) {
		dev_err(&pdev->dev, "ioremap() of registers failed\n");
		retval = -ENXIO;
		goto release_mem;
	}
    
	/* bit[14]  : 1-A/D converter prescaler enable
	 * bit[13:6]: A/D converter prescaler value,
	 *            49, ADCCLK=PCLK/(49+1)=50MHz/(49+1)=1MHz
	 * bit[0]: A/D conversion starts by enable. ����Ϊ0
	 */
    tmp = (1<<14)|(49<<6);
    writel(tmp, ts.ts_virt + S3C2410_ADCCON);

    ts.res_irq_tc = platform_get_resource(pdev, IORESOURCE_IRQ, 0);
    if (NULL == ts.res_irq_tc) {
        dev_err(&pdev->dev, "failed to get irq registers\n");
        retval = -ENXIO;
        goto release_mem;
    }

    if ((retval = request_irq(ts.res_irq_tc->start, pen_down_up_irq, IRQF_SAMPLE_RANDOM, \
                "ts_pen", NULL)) < 0)
        goto release_mem;

    ts.res_irq_adc = platform_get_resource(pdev, IORESOURCE_IRQ, 1);
    if (NULL == ts.res_irq_tc) {
        dev_err(&pdev->dev, "failed to get irq registers\n");
        retval = -ENXIO;
        goto release_irq_tc;
    }

    if ((retval = request_irq(ts.res_irq_adc->start, adc_irq, IRQF_SAMPLE_RANDOM, \
                "adc", NULL)) < 0)
        goto release_irq_tc;

    
	/* �Ż���ʩ1: 
	 * ����ADCDLYΪ���ֵ, ��ʹ�õ�ѹ�ȶ����ٷ���IRQ_TC�ж�
	 */    
    writel(0xffff, ts.ts_virt + S3C2410_ADCDLY);

	/* �Ż���ʩ5: ʹ�ö�ʱ��������,���������
	 * 
	 */
	init_timer(&ts.ts_timer);
	ts.ts_timer.function = s3c_ts_timer_function;
	add_timer(&ts.ts_timer);

	enter_wait_pen_down_mode();
    
    return 0;

release_irq_tc:
    free_irq(ts.res_irq_tc->start, NULL);
release_mem:
    release_mem_region(ts.res_mem->start, size);
fail_get_res:
    clk_disable(ts.clk);
dealloc_input_dev:
    input_free_device(ts.s3c_ts_dev);
input_alloc_fail:    
    return retval;
}


static int mini2440_ts_drv_remove(struct platform_device *pdev)
{
	free_irq(ts.res_irq_tc->start, NULL);
	free_irq(ts.res_irq_adc->start, NULL);
	iounmap(ts.ts_virt);
    release_mem_region(ts.res_mem->start, resource_size(ts.res_mem));    
    clk_disable(ts.clk);
	input_unregister_device(ts.s3c_ts_dev);
	input_free_device(ts.s3c_ts_dev);
	del_timer(&ts.ts_timer);
    return 0;
}


static struct platform_driver mini2440_ts_driver = {
    .probe              = mini2440_ts_drv_probe,
    .remove             = __devexit_p(mini2440_ts_drv_remove),
    .driver             = {
        .owner  = THIS_MODULE,
        .name   = "mini2440-ts",
    },
};


static int __init mini2440_ts_drv_init(void)
{
    return platform_driver_register(&mini2440_ts_driver);
}

static void __exit mini2440_ts_drv_exit(void)
{
    platform_driver_unregister(&mini2440_ts_driver);
}

module_init(mini2440_ts_drv_init);
module_exit(mini2440_ts_drv_exit);

