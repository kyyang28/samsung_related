
#include <linux/kernel.h>
#include <linux/module.h>

#include <linux/i2c.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/interrupt.h>
#include <linux/delay.h>
#include <linux/errno.h>
#include <linux/err.h>
#include <linux/platform_device.h>
#include <linux/pm_runtime.h>
#include <linux/clk.h>
#include <linux/cpufreq.h>
#include <linux/slab.h>
#include <linux/io.h>
#include <linux/of_i2c.h>
#include <linux/of_gpio.h>
#include <plat/gpio-cfg.h>
#include <mach/regs-gpio.h>
#include <mach/gpio-bank-b.h>
#include <mach/irqs.h>

#include <asm/irq.h>

#include <plat/regs-iic.h>
#include <plat/iic.h>

//#define AT24CXX_DBG   printk
#define AT24CXX_DBG(...) 

enum s3c24xx_i2c_state {
	STATE_IDLE,
	STATE_START,
	STATE_READ,
	STATE_WRITE,
	STATE_STOP
};


struct s3c6410_i2c_xfer_data {
	struct i2c_msg *msgs;
	int msn_num;
	int cur_msg;
	int cur_ptr;
	int state;
	int err;
	wait_queue_head_t wait;
	struct resource	 *ioarea;
    void __iomem *regs;
    int irq_base;
};

static struct s3c6410_i2c_xfer_data s3c6410_i2c_xfer_data;


static void s3c6410_i2c_start(void)
{
    unsigned char byte;
    unsigned int i2cstat;
	s3c6410_i2c_xfer_data.state = STATE_START;
	
	if (s3c6410_i2c_xfer_data.msgs->flags & I2C_M_RD) { /* �� */ 
        byte = s3c6410_i2c_xfer_data.msgs->addr << 1;
        writeb(byte, s3c6410_i2c_xfer_data.regs + S3C2410_IICDS);
		//s3c6410_i2c_regs->iicds		 = s3c6410_i2c_xfer_data.msgs->addr << 1;

        /* i2cstat = 0xb0 */
        i2cstat = S3C2410_IICSTAT_MASTER_RX | S3C2410_IICSTAT_START | S3C2410_IICSTAT_TXRXEN;
        writel(i2cstat, s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
        //s3c6410_i2c_regs->iicstat 	 = 0xb0;	// �������գ�����
	}else { /* д */
        byte = s3c6410_i2c_xfer_data.msgs->addr << 1;
        writeb(byte, s3c6410_i2c_xfer_data.regs + S3C2410_IICDS);
		//s3c6410_i2c_regs->iicds		 = s3c6410_i2c_xfer_data.msgs->addr << 1;
        
        /* i2cstat = 0xf0 */
        i2cstat = S3C2410_IICSTAT_MASTER_TX | S3C2410_IICSTAT_START | S3C2410_IICSTAT_TXRXEN;
        writel(i2cstat, s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
        //s3c6410_i2c_regs->iicstat    = 0xf0; 		// �������ͣ�����
	}
}

static void s3c6410_i2c_stop(int err)
{
    unsigned int i2cstat;
    unsigned long tmp;
	s3c6410_i2c_xfer_data.state = STATE_STOP;
	s3c6410_i2c_xfer_data.err   = err;

	AT24CXX_DBG("STATE_STOP, err = %d\n", err);


	if (s3c6410_i2c_xfer_data.msgs->flags & I2C_M_RD) { /* �� */
		// �������лָ�I2C����������P�ź�
		//s3c6410_i2c_regs->iicstat = 0x90;
		i2cstat = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
		i2cstat = S3C2410_IICSTAT_MASTER_RX | S3C2410_IICSTAT_TXRXEN;
        writel(i2cstat, s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);

        tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
        /* tmp = 0xaf */
        tmp = S3C2410_IICCON_ACKEN | S3C2410_IICCON_TXDIV_16 | S3C2410_IICCON_IRQEN | S3C2410_IICCON_SCALEMASK;  
        writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
        //s3c6410_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // �ȴ�һ��ʱ���Ա�P�ź��Ѿ�����
	}else { /* д */
		// �������������ָ�I2C����������P�ź�
		i2cstat = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
		i2cstat = S3C2410_IICSTAT_MASTER_TX | S3C2410_IICSTAT_TXRXEN;
        writel(i2cstat, s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
        //s3c6410_i2c_regs->iicstat = 0xd0;

        tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
        /* tmp = 0xaf */
        tmp = S3C2410_IICCON_ACKEN | S3C2410_IICCON_TXDIV_16 | S3C2410_IICCON_IRQEN | S3C2410_IICCON_SCALEMASK;  
        writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
        //s3c6410_i2c_regs->iiccon  = 0xaf;
		ndelay(50);  // �ȴ�һ��ʱ���Ա�P�ź��Ѿ�����
	}

	/* ���� */
	wake_up(&s3c6410_i2c_xfer_data.wait);
}

static int s3c6410_i2c_xfer(struct i2c_adapter *adap,
			struct i2c_msg *msgs, int num)
{
	unsigned long timeout;
	
	/* ��num��msg��I2C���ݷ��ͳ�ȥ/������ */
	s3c6410_i2c_xfer_data.msgs    = msgs;
	s3c6410_i2c_xfer_data.msn_num = num;
	s3c6410_i2c_xfer_data.cur_msg = 0;
	s3c6410_i2c_xfer_data.cur_ptr = 0;
	s3c6410_i2c_xfer_data.err     = -ENODEV;

	s3c6410_i2c_start();

	/* ���� */
	timeout = wait_event_timeout(s3c6410_i2c_xfer_data.wait, (s3c6410_i2c_xfer_data.state == STATE_STOP), HZ * 5);
	if (0 == timeout) {
		printk("s3c6410_i2c_xfer time out\n");
		return -ETIMEDOUT;
	}else {
		return s3c6410_i2c_xfer_data.err;
	}
}

static u32 s3c6410_i2c_func(struct i2c_adapter *adap)
{
	return I2C_FUNC_I2C | I2C_FUNC_SMBUS_EMUL | I2C_FUNC_PROTOCOL_MANGLING;
}


static const struct i2c_algorithm s3c6410_i2c_algo = {
	//.smbus_xfer     = s3c6410_smbus_xfer,
	.master_xfer	= s3c6410_i2c_xfer,
	.functionality	= s3c6410_i2c_func,
};

/* 1. ����/����i2c_adapter
 */
static struct i2c_adapter s3c6410_i2c_adapter = {
     .name			 = "s3c6410_i2cbus",
     .algo			 = &s3c6410_i2c_algo,
     .owner 		 = THIS_MODULE,
};

static int isLastMsg(void)
{
	return (s3c6410_i2c_xfer_data.cur_msg == s3c6410_i2c_xfer_data.msn_num - 1);
}

static int isEndData(void)
{
	return (s3c6410_i2c_xfer_data.cur_ptr >= s3c6410_i2c_xfer_data.msgs->len);
}

static int isLastData(void)
{
	return (s3c6410_i2c_xfer_data.cur_ptr == s3c6410_i2c_xfer_data.msgs->len - 1);
}

static irqreturn_t s3c6410_i2c_xfer_irq(int irq, void *dev_id)
{
    unsigned char byte;
    unsigned long tmp;
	unsigned int iicSt = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
    //iicSt  = s3c6410_i2c_regs->iicstat; 

	if(iicSt & S3C2410_IICSTAT_ARBITR) 
        printk("Bus arbitration failed\n\r");

	switch (s3c6410_i2c_xfer_data.state) {
    case STATE_START : /* ����S���豸��ַ��,�����ж� */
	{
		AT24CXX_DBG("Start\n");
		/* ���û��ACK, ���ش��� */
		if (iicSt & S3C2410_IICSTAT_LASTBIT) {
			s3c6410_i2c_stop(-ENODEV);
			break;
		}

		if (isLastMsg() && isEndData()) {
			s3c6410_i2c_stop(0);
			break;
		}

		/* ������һ��״̬ */
		if (s3c6410_i2c_xfer_data.msgs->flags & I2C_M_RD) { /* �� */ 
			s3c6410_i2c_xfer_data.state = STATE_READ;
			goto next_read;
		}else {
			s3c6410_i2c_xfer_data.state = STATE_WRITE;
		}	
	}

	case STATE_WRITE: {
		AT24CXX_DBG("STATE_WRITE\n");
		/* ���û��ACK, ���ش��� */
		if (iicSt & S3C2410_IICSTAT_LASTBIT) {
			s3c6410_i2c_stop(-ENODEV);
			break;
		}

		if (!isEndData()) { /* �����ǰmsg��������Ҫ���� */
            byte = s3c6410_i2c_xfer_data.msgs->buf[s3c6410_i2c_xfer_data.cur_ptr];
            writeb(byte, s3c6410_i2c_xfer_data.regs + S3C2410_IICDS);
			//s3c6410_i2c_regs->iicds = s3c6410_i2c_xfer_data.msgs->buf[s3c6410_i2c_xfer_data.cur_ptr];
			s3c6410_i2c_xfer_data.cur_ptr++;
			
			// ������д��IICDS����Ҫһ��ʱ����ܳ�����SDA����
			ndelay(50);	

            tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
            /* tmp = 0xaf */
            tmp = S3C2410_IICCON_ACKEN | S3C2410_IICCON_TXDIV_16 | S3C2410_IICCON_IRQEN | S3C2410_IICCON_SCALEMASK;  
            writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
			//s3c6410_i2c_regs->iiccon = 0xaf;		// �ָ�I2C����
			break;				
		}else if (!isLastMsg()) {
			/* ��ʼ������һ����Ϣ */
			s3c6410_i2c_xfer_data.msgs++;
			s3c6410_i2c_xfer_data.cur_msg++;
			s3c6410_i2c_xfer_data.cur_ptr = 0;
			s3c6410_i2c_xfer_data.state = STATE_START;
			/* ����START�źźͷ����豸��ַ */
			s3c6410_i2c_start();
			break;
		}else {
			/* �����һ����Ϣ�����һ������ */
			s3c6410_i2c_stop(0);
			break;				
		}

		break;
	}

	case STATE_READ: {
		AT24CXX_DBG("STATE_READ\n");
		/* �������� */
		//s3c6410_i2c_xfer_data.msgs->buf[s3c6410_i2c_xfer_data.cur_ptr] = s3c6410_i2c_regs->iicds;			
        byte = readb(s3c6410_i2c_xfer_data.regs + S3C2410_IICDS);
        s3c6410_i2c_xfer_data.msgs->buf[s3c6410_i2c_xfer_data.cur_ptr] = byte;			
		s3c6410_i2c_xfer_data.cur_ptr++;
next_read:
		if (!isEndData()) { /* �������û��д, ������������� */
			if (isLastData()) { /* ��������������������һ��, ����ack */
                tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
                /* tmp = 0x2f */
                tmp = S3C2410_IICCON_IRQEN | S3C2410_IICCON_SCALEMASK;
                writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
                //s3c6410_i2c_regs->iiccon = 0x2f;   // �ָ�I2C���䣬���յ���һ����ʱ��ACK
			}else {
                tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);                    
                /* tmp = 0xaf */
                tmp = S3C2410_IICCON_ACKEN | S3C2410_IICCON_TXDIV_16 | S3C2410_IICCON_IRQEN | S3C2410_IICCON_SCALEMASK;  
                writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
				//s3c6410_i2c_regs->iiccon = 0xaf;   // �ָ�I2C���䣬���յ���һ����ʱ����ACK
			}				
			break;
		}else if (!isLastMsg()) {
			/* ��ʼ������һ����Ϣ */
			s3c6410_i2c_xfer_data.msgs++;
			s3c6410_i2c_xfer_data.cur_msg++;
			s3c6410_i2c_xfer_data.cur_ptr = 0;
			s3c6410_i2c_xfer_data.state = STATE_START;
			/* ����START�źźͷ����豸��ַ */
			s3c6410_i2c_start();
			break;
		}else {
			/* �����һ����Ϣ�����һ������ */
			s3c6410_i2c_stop(0);
			break;								
		}
		break;
	}

	default: break;
	}

	/* ���ж� */
    tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
    writel(tmp & ~(S3C2410_IICCON_IRQPEND), s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
	//s3c6410_i2c_regs->iiccon &= ~(S3C2410_IICCON_IRQPEND);

	return IRQ_HANDLED;	
}


/*
 * I2C��ʼ��
 */
static void s3c6410_i2c_init(void)
{
	struct clk *clk;
    unsigned long tmp;

	clk = clk_get(NULL, "i2c");
	clk_enable(clk);
	
    // ѡ�����Ź��ܣ�GPE15:IICSDA, GPE14:IICSCL
	s3c_gpio_cfgpin(S3C64XX_GPB(5), S3C64XX_GPB5_I2C_SCL0);
	s3c_gpio_cfgpin(S3C64XX_GPB(6), S3C64XX_GPB6_I2C_SDA0);

    /* bit[7] = 1, ʹ��ACK
     * bit[6] = 0, IICCLK = PCLK/16
     * bit[5] = 1, ʹ���ж�
     * bit[3:0] = 0xf, Tx clock = IICCLK/16
     * PCLK = 50MHz, IICCLK = 3.125MHz, Tx Clock = 0.195MHz
     */
    tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
    tmp = S3C2410_IICCON_ACKEN | S3C2410_IICCON_TXDIV_16 | S3C2410_IICCON_IRQEN | S3C2410_IICCON_SCALEMASK;  
    writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICCON);
    //s3c6410_i2c_regs->iiccon = (1<<7) | (0<<6) | (1<<5) | (0xf);  // 0xaf

    tmp = readb(s3c6410_i2c_xfer_data.regs + S3C2410_IICADD);
    tmp = 0x10; /* slave_addr */
    writeb(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICADD);
    //s3c6410_i2c_regs->iicadd  = 0x10;     // S3C24xx slave address = [7:1]

    tmp = readl(s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
    tmp = 0x10;
    writel(tmp, s3c6410_i2c_xfer_data.regs + S3C2410_IICSTAT);
    //s3c6410_i2c_regs->iicstat = 0x10;     // I2C�������ʹ��(Rx/Tx)
}


static int s3c6410_i2c_bus_probe(struct platform_device *pdev)
{    
    /* 2. Ӳ����ص����� */
	struct resource *res;

	res = platform_get_resource(pdev, IORESOURCE_MEM, 0);
    if (!res) {
        printk("[s3c6410_i2c_bus]Error: Failed to get IORESOURCE_MEM!\n");
        return -ENODEV;
    }

    s3c6410_i2c_xfer_data.ioarea = request_mem_region(res->start, resource_size(res), pdev->name);
    if (s3c6410_i2c_xfer_data.ioarea == NULL) {        
        printk("[s3c6410_i2c_bus]Error: Failed to request iomem!\n");
        return -ENOMEM;
    }

    s3c6410_i2c_xfer_data.regs = ioremap(res->start, resource_size(res));
    if (s3c6410_i2c_xfer_data.regs == NULL) {
        printk("[s3c6410_i2c_bus]Error: Failed to ioremap io regs!\n");
        return -ENOMEM;
    }
	
	s3c6410_i2c_init();

    s3c6410_i2c_xfer_data.irq_base = platform_get_irq(pdev, 0);
    if (s3c6410_i2c_xfer_data.irq_base < 0) {
        printk("[s3c6410_i2c_bus]Error: Failed to get irq!\n");
        return -ENODEV;
    }

    if (request_irq(s3c6410_i2c_xfer_data.irq_base, s3c6410_i2c_xfer_irq, 0, "s3c6410-i2c", NULL)) {
    	printk(KERN_ERR "[i2c_bus_s3c6410]cannot obtain IRQ %d\n", s3c6410_i2c_xfer_data.irq_base);
        return -EBUSY;
    }

	init_waitqueue_head(&s3c6410_i2c_xfer_data.wait);
	
	/* 3. ע��i2c_adapter */
	i2c_add_adapter(&s3c6410_i2c_adapter);

    return 0;
}

static int s3c6410_i2c_bus_remove(struct platform_device *pdev)
{    
	i2c_del_adapter(&s3c6410_i2c_adapter);	
	free_irq(s3c6410_i2c_xfer_data.irq_base, NULL);
	iounmap(s3c6410_i2c_xfer_data.regs);
    return 0;
}


static struct platform_driver s3c6410_i2c_bus_plat_drv = {
	.driver		= {
		.name	= "i2c_s3c6410",
		.owner	= THIS_MODULE,
	},
	.probe		= s3c6410_i2c_bus_probe,
	.remove		= __devexit_p(s3c6410_i2c_bus_remove),
};


static int i2c_bus_s3c6410_init(void)
{    
    platform_driver_register(&s3c6410_i2c_bus_plat_drv);
	return 0;
}

static void i2c_bus_s3c6410_exit(void)
{
    platform_driver_unregister(&s3c6410_i2c_bus_plat_drv);
}

module_init(i2c_bus_s3c6410_init);
module_exit(i2c_bus_s3c6410_exit);
MODULE_LICENSE("GPL");


