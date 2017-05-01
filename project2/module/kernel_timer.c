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
int device_ioctl(struct inode *, struct file *, unsigned int, unsigned long);

static struct file_operations kernel_timer_fops =
{
  .open = kernel_timer_open,
  .write = kernel_timer_write,
  .release = kernel_timer_release,
  .ioctl = device_ioctl
};

static struct struct_mydata 
{
  struct timer_list timer;
  int count;
};

struct struct_mydata mydata;

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
int
device_ioctl(struct inode *inode,
             struct file *file,
             unsigned int ioctl_num,/* The number of the ioctl */
             unsigned long ioctl_param) /* The parameter to it */
{
  int i;
  char *temp;
  char ch;

  /* Switch according to the ioctl called */
  switch (ioctl_num) {
    case IOCTL_SET_MSG:
      /* Receive a pointer to a message (in user space) 
       * and set that to be the device's message. */ 

      /* Get the parameter given to ioctl by the process */
      temp = (char *) ioctl_param;

      /* Find the length of the message */
      get_user(ch, temp);
      for (i=0; ch && i<BUF_LEN; i++, temp++)
        get_user(ch, temp);

      /* Don't reinvent the wheel - call device_write */
      device_write(file, (char *) ioctl_param, i, 0);
      break;

    case IOCTL_GET_MSG:
      /* Give the current message to the calling 
       * process - the parameter we got is a pointer, 
       * fill it. */
      i = device_read(file, (char *) ioctl_param, 99, 0); 
      /* Warning - we assume here the buffer length is 
       * 100. If it's less than that we might overflow 
       * the buffer, causing the process to core dump. 
       *
       * The reason we only allow up to 99 characters is 
       * that the NULL which terminates the string also 
       * needs room. */

      /* Put a zero at the end of the buffer, so it 
       * will be properly terminated */
      put_user('\0', (char *) ioctl_param+i);
      break;

    case IOCTL_GET_NTH_BYTE:
      /* This ioctl is both input (ioctl_param) and 
       * output (the return value of this function) */
      return Message[ioctl_param];
      break;
  }

  return SUCCESS;
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
