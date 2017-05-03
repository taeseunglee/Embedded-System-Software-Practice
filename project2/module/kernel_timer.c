#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

/* ioctl numbers */
#include "./kernel_timer.h"

#define __POWER_OF_0 1
#define __POWER_OF_1 10
#define __POWER_OF_2 100
#define __POWER_OF_3 1000
#define POWER_OF(NUM) __POWER_OF_ ## NUM


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
  int pat_fnd;
  int base_fnd;
};

static struct struct_mydata mydata;
static unsigned long t_interval;
static int dot_fd;
static int led_fd;
static int fnd_fd;
static int lcd_fd;


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

  ++ p_data->count;
  if( p_data->count > 15 )
    return;

  mydata.timer.expires = get_jiffies_64() + t_interval;
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function = kernel_timer_blink;

  add_timer(&mydata.timer);
}

ssize_t
kernel_timer_write(struct file *file, const char *gdata, size_t length, loff_t *off_what)
{
  const lonh *tmp = gdata;
  long kernel_timer_buff = 0;
  long loc_fnd;

  if ( copy_from_user(&kernel_timer_buff, tmp, sizeof(long)) )
    return -EFAULT;

  /* modify HZ to interval after I set timer interval at interval variable */
  mydata.count    = kernel_timer_buff & 0xFF;
  mydata.pat_fnd  = (kernel_timer_buff & 0xFF0000) >> 16;
  t_interval  = (unsigned long)((kernel_timer_buff & 0xFF00) >> 8) * HZ / 10;
  loc_fnd     = (kernel_timer_buff & 0xFF000000) >> 24;

  mydata.base_fnd = 1000 / POWER_OF(loc_fnd);

  del_timer_sync(&mydata.timer);

  mydata.timer.expires  = jiffies_64 + t_interval;
  mydata.timer.data     = (unsigned long)&mydata;
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

/* ioctl_num : The number of the ioctl */
/* ioctl_param : The parameter to it */
long
kernel_timer_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  int i;
  char ch;

  /* Switch according to the ioctl called */
  switch (ioctl_num)
    {
    case KTIMER_SET_FND:
      fnd_fd = (int) ioctl_param;
      break;

    case KTIMER_SET_LED:
      led_fd = (int) ioctl_param;
      break;

    case KTIMER_SET_DOT:
      dot_fd = (int) ioctl_param;
      break;

    case KTIMER_SET_LCD:
      lcd_fd = (int) ioctl_param;
      break;
    case KTIMER_START:
      kernel_timer_write(file, (char *) ioctl_param, (size_t) 0, (loff_t *) 0);
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
