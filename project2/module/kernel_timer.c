#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

/* ioctl numbers */
#include "./kernel_timer.h"


static int kernel_timer_usage = 0;

int kernel_timer_open(struct inode *, struct file *);
int kernel_timer_release(struct inode *, struct file *);
ssize_t kernel_timer_write(struct file *, const char *, size_t, loff_t *);
long kernel_timer_ioctl(struct file *, unsigned int, unsigned long);

static struct file_operations kernel_timer_fops =
{
  .open = kernel_timer_open,
  .write = kernel_timer_write,
  .release = kernel_timer_release,
  .unlocked_ioctl = kernel_timer_ioctl,
};

struct struct_mydata 
{
  struct timer_list timer;
  int count;
  int pattern;
};

static struct struct_mydata mydata;
static int interval;


int
kernel_timer_release(struct inode *minode, struct file *mfile)
{
  printk("kernel_timer_release\n");
  kernel_timer_usage = 0;
  return 0;
}

int
kernel_timer_open(struct inode *minode, struct file *mfile)
{
  printk("kernel_timer_open\n");
  if (kernel_timer_usage != 0)
    return -EBUSY;
  kernel_timer_usage = 1;
  return 0;
}

static void
kernel_timer_blink(unsigned long timeout)
{
  struct struct_mydata *p_data = (struct struct_mydata*)timeout;

  printk("kernel_timer_blink %d\n", p_data->count);

  p_data->count++;
  if( p_data->count > 15 )
    return;

  mydata.timer.expires = get_jiffies_64() + (1 * HZ);
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function = kernel_timer_blink;

  add_timer(&mydata.timer);
}

ssize_t
kernel_timer_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what)
{
  const char *tmp = gdata;
  char kernel_timer_buff = 0;

  printk("write\n");
  // 1 byte
  if (copy_from_user(&kernel_timer_buff, tmp, 1))
    return -EFAULT;

  mydata.count = kernel_timer_buff;
  /* modify HZ to interval after I set timer interval at interval variable */

  printk("data  : %d \n",mydata.count);

  del_timer_sync(&mydata.timer);

  mydata.timer.expires = jiffies + (1 * HZ);
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function	= kernel_timer_blink;

  add_timer(&mydata.timer);
  return 1;
}

/* This function is called whenever a process tries to 
 * do an ioctl on our device file. We get two extra 
 * parameters (additional to the inode and file 
 * structures, which all device functions get): the number
 * of the ioctl called and the parameter given to the 
 * ioctl function.
 *
 * If the ioctl is write or read/write (meaning output 
 * is returned to the calling process), the ioctl call 
 * returns the output of this function.
 */
long
kernel_timer_ioctl(struct file *file,
                   unsigned int ioctl_num,/* The number of the ioctl */
                   unsigned long ioctl_param) /* The parameter to it */
{
  int i;
  int temp = (int) ioctl_param;
  char ch;

  /* Switch according to the ioctl called */
  switch (ioctl_num)
    {
    case KTIMER_SET_FND:
      break;

    case KTIMER_SET_LED:
      break;

    case KTIMER_SET_DOT:
      break;

    case KTIMER_SET_LCD:
      break;
    }

  return 0;
}

int __init 
kernel_timer_init(void)
{
  int result;

  struct class *kernel_timer_dev_class=NULL;
  struct device *kernel_timer_dev=NULL;

  printk("kernel_timer_init\n");


  result = register_chrdev(KERNEL_TIMER_MAJOR,
                           KERNEL_TIMER_NAME,
                           &kernel_timer_fops);
  if(result <0)
    {
      printk( "error %d\n",result);
      return result;
    }

  kernel_timer_dev = device_create(kernel_timer_dev_class,
                                   NULL,
                                   MKDEV(KERNEL_TIMER_MAJOR , 0),
                                   NULL,
                                   KERNEL_TIMER_NAME);

  init_timer(&(mydata.timer));

  printk("init module\n");
  return 0;
}

void __exit
kernel_timer_exit(void)
{
  printk("kernel_timer_exit\n");
  kernel_timer_usage = 0;
  del_timer_sync(&mydata.timer);

  unregister_chrdev(KERNEL_TIMER_MAJOR, KERNEL_TIMER_NAME);
}

module_init( kernel_timer_init);
module_exit( kernel_timer_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
