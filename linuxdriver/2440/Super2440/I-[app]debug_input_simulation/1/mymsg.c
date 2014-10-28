

/*  
 * Referenced by 
 *      d:\embedded\linux_kernel\linux-2.6.38_r23\linux-2.6.38\fs\proc\kmsg.c
 */

#include <linux/module.h>  
#include <linux/types.h>
#include <linux/errno.h>
#include <linux/time.h>
#include <linux/kernel.h>
#include <linux/poll.h>
#include <linux/proc_fs.h>
#include <linux/fs.h>
#include <linux/syslog.h>
#include <linux/sched.h>

#include <asm/uaccess.h>
#include <asm/io.h>

#include <stdarg.h>


#define MYLOG_BUF_SIZE     (1024*1024)

//#define DBG_PRINTK          printk
#define DBG_PRINTK(x...)    

struct proc_dir_entry *mymsg_entry;

//static char mylog_buf[MYLOG_BUF_SIZE];
static char tmp_buf[1024];

static char *mylog_buf;

static int mylog_read = 0;      /* mylog_start */
static int mylog_write = 0;     /* mylog_end */
static int mylog_read_flag = 0; 

static DECLARE_WAIT_QUEUE_HEAD(mymsg_waitq);


static int is_mylog_empty(void)
{
    return (mylog_read == mylog_write);
}

static int is_mylog_empty_flag(void)
{
    return (mylog_read_flag == mylog_write);
}

static int is_mylog_full(void)
{
    return (((mylog_write + 1) % MYLOG_BUF_SIZE) == mylog_read);
}

static void mylog_putc(char ch)
{
    if (is_mylog_full()) {
        mylog_read = (mylog_read + 1) % MYLOG_BUF_SIZE;

        if (((mylog_read_flag + 1) % MYLOG_BUF_SIZE) == mylog_read)
            mylog_read_flag = mylog_read;
    }

    mylog_buf[mylog_write] = ch;    
    mylog_write = (mylog_write + 1) % MYLOG_BUF_SIZE;

    wake_up_interruptible(&mymsg_waitq);
}

static int mylog_getc(char *p)
{
    if (is_mylog_empty())
        return 0;   /* 0 means empty */

    *p = mylog_buf[mylog_read];
    mylog_read = (mylog_read + 1) % MYLOG_BUF_SIZE;
    return 1;   /* 1 means success */
}

static int mylog_getc_flag(char *p)
{
    if (is_mylog_empty_flag())
        return 0;   /* 0 means empty */

    *p = mylog_buf[mylog_read_flag];
    mylog_read_flag = (mylog_read_flag + 1) % MYLOG_BUF_SIZE;
    return 1;   /* 1 means success */
}

int myprintk(const char *fmt, ...)
{
	va_list args;
	int i, j;

	va_start(args, fmt);
	i = vsnprintf(tmp_buf, INT_MAX, fmt, args);
	va_end(args);

    for (j = 0; j < i; j++)
        mylog_putc(tmp_buf[j]);

	return i;
}

static int mymsg_open(struct inode *inode, struct file *file)
{
    mylog_read_flag = mylog_read;

    return 0;
}


static ssize_t mymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{    
    int i = 0;
	int error = 0;
    char ch;

	if ((file->f_flags & O_NONBLOCK) && is_mylog_empty_flag())
		return -EAGAIN;

    DBG_PRINTK("%s %d\n", __FUNCTION__, __LINE__);
    DBG_PRINTK("count = %d\n", count);
    DBG_PRINTK("mylog_read = %d\n", mylog_read);
    DBG_PRINTK("mylog_write = %d\n", mylog_write);

    error = wait_event_interruptible(mymsg_waitq, !is_mylog_empty_flag());

    DBG_PRINTK("%s %d\n", __FUNCTION__, __LINE__);
    DBG_PRINTK("count = %d\n", count);
    DBG_PRINTK("mylog_read = %d\n", mylog_read);
    DBG_PRINTK("mylog_write = %d\n", mylog_write);

    /* copy_to_user */
    while (!error && mylog_getc_flag(&ch) && i < count) {
        error = __put_user(ch, buf);
        buf++;
        i++;
    }    

    if (!error)
        error = i;

    return error;
}


static const struct file_operations proc_mymsg_fops = {
    .open = mymsg_open,
    .read = mymsg_read,
};

static int __init mymsg_init(void)
{
    mylog_buf = kmalloc(MYLOG_BUF_SIZE, GFP_KERNEL);
    if (!mylog_buf) {
        printk("Charles.Y[drivers]: Unable to allocate mylog_buf\n");
        return -EIO;
    }
    
    //proc_create("mymsg", S_IRUSR, NULL, &proc_mymsg_fops);
    mymsg_entry = create_proc_entry("mymsg", S_IRUSR, NULL);
    if (mymsg_entry)
        mymsg_entry->proc_fops = &proc_mymsg_fops;    

    return 0;
}

static void __exit mymsg_exit(void)
{
    remove_proc_entry("mymsg", NULL);
    kfree(mylog_buf);
}

module_init(mymsg_init);
module_exit(mymsg_exit);

EXPORT_SYMBOL(myprintk);

MODULE_LICENSE("GPL");


