

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

#include <asm/uaccess.h>
#include <asm/io.h>

#if 0
extern wait_queue_head_t log_wait;

static int mymsg_open(struct inode * inode, struct file * file)
{
	return do_syslog(SYSLOG_ACTION_OPEN, NULL, 0, SYSLOG_FROM_FILE);
}

static int mymsg_release(struct inode * inode, struct file * file)
{
	(void) do_syslog(SYSLOG_ACTION_CLOSE, NULL, 0, SYSLOG_FROM_FILE);
	return 0;
}

static ssize_t mymsg_read(struct file *file, char __user *buf,
			 size_t count, loff_t *ppos)
{
	if ((file->f_flags & O_NONBLOCK) &&
	    !do_syslog(SYSLOG_ACTION_SIZE_UNREAD, NULL, 0, SYSLOG_FROM_FILE))
		return -EAGAIN;
	return do_syslog(SYSLOG_ACTION_READ, buf, count, SYSLOG_FROM_FILE);
}

static unsigned int mymsg_poll(struct file *file, poll_table *wait)
{
	poll_wait(file, &log_wait, wait);
	if (do_syslog(SYSLOG_ACTION_SIZE_UNREAD, NULL, 0, SYSLOG_FROM_FILE))
		return POLLIN | POLLRDNORM;
	return 0;
}
#endif

struct proc_dir_entry *mymsg_entry;

static const struct file_operations proc_mymsg_fops = { 
};

static int __init mymsg_init(void)
{
    //proc_create("mymsg", S_IRUSR, NULL, &proc_mymsg_fops);
#if 1
    mymsg_entry = create_proc_entry("mymsg", S_IRUSR, NULL);
    if (mymsg_entry)
        mymsg_entry->proc_fops = &proc_mymsg_fops;    
#endif
    return 0;
}

static void __exit mymsg_exit(void)
{
    remove_proc_entry("mymsg", NULL);
}

module_init(mymsg_init);
module_exit(mymsg_exit);

MODULE_LICENSE("GPL");


