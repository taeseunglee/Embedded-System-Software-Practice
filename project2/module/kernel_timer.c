#include <linux/string.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

/* ioctl numbers */
#include "./kernel_timer.h"

#include "./fpga_dot/fpga_dot_driver.c"
#include "./fpga_fnd/fpga_fnd_driver.c"
#include "./fpga_led/fpga_led_driver.c"
#include "./fpga_text_lcd/fpga_text_lcd_driver.c"

#include <asm/unistd.h>

/*
#define __LOC_VAL3 1
#define __LOC_VAL2 10
#define __LOC_VAL1 100
#define __LOC_VAL0 1000
#define FND_LOC_VALUE(NUM) __LOC_VAL ## NUM
*/

/* shift right */
#define __SHIFT_TEXT_0(str, idx) do { \
    idx = 15; \
    do { \
      str[idx] = str[idx-1]; \
    } while (--idx); \
    str[0] = 0; \
  } while (0);

/* shift left */
#define __SHIFT_TEXT_1(str, idx) do { \
    idx = 0; \
    do { \
      str[idx] = str[idx+1]; \
      ++idx; \
    } while(idx < 15); \
    str[15] = 0; \
  } while(0);
 


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
  int loc_fnd;
};

static struct struct_mydata mydata;
static unsigned long t_interval;
static int dot_fd;
static int led_fd;
static int fnd_fd;
static int lcd_fd;


/* stn : student number */
/* direction: 0 - right, 1 - left */
static char stn[16] = "20141570";
static char name[16] = "Taeseung Lee";
static int len_stn;
static int len_name;
static int remained_stn_cnt;
static int remained_name_cnt;
static int direction_stn;
static int direction_name;

void
kernel_timer_device_set(struct file * file)
{
  int idx = 0;

  /* TODO: Error check */
  /** set devices **/
  char dev_data[33];

  /* set fnd device */
  memset(dev_data, 0, 4);
  dev_data[mydata.loc_fnd] = mydata.pat_fnd;
  iom_fpga_fnd_fops.write(file, dev_data, 4, 0);

  /* set led device */
  dev_data[0] = 256 >> mydata.pat_fnd;
  iom_led_fops.write(file, dev_data, 1, 0);

  /* set dot device */
  memcpy(dev_data, fpga_number[mydata.pat_fnd], 10);
  iom_fpga_dot_fops.write(file, dev_data, 10, 0);

  /* set text lcd */
  direction_stn     = direction_name = 0;
  remained_stn_cnt  = 16 - (len_stn = strlen(stn));
  remained_name_cnt = 16 - (len_name = strlen(name));

  memcpy(dev_data, stn, 16*sizeof(char));
  memcpy(dev_data+16, name, 16*sizeof(char));
  iom_fpga_text_lcd_fops.write(file, dev_data, 32, 0);

  // suppose strlen < 16 --> TODO: Need a generalization
  if (remained_stn_cnt)
    -- remained_stn_cnt;
  else
    {
      remained_stn_cnt = 16 - len_stn - 1;
      direction_stn ^= 1;
    }

  if (direction_stn) { __SHIFT_TEXT_1(stn, idx); }
  else { __SHIFT_TEXT_0(stn, idx); }


  if (remained_name_cnt)
    -- remained_name_cnt;
  else
    {
      remained_name_cnt = 16 - len_name - 1;
      direction_name ^= 1;
    }

  if (direction_name) { __SHIFT_TEXT_1(name, idx); }
  else { __SHIFT_TEXT_0(name, idx); }
}

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

  iom_fpga_dot_fops.open(minode, mfile);
  iom_fpga_fnd_fops.open(minode, mfile);
  iom_led_fops.open(minode, mfile);
  iom_fpga_text_lcd_fops.open(minode, mfile);

  return 0;
}

static void
kernel_timer_blink(unsigned long timeout)
{
//  struct struct_mydata *p_data = (struct struct_mydata*)timeout;

//  printk("kernel_timer_blink %d\n", p_data->count);

//  if( --p_data->count )
//    return;

  if (-- mydata.count)
    return ;


  kernel_timer_device_set();

  mydata.timer.expires = get_jiffies_64() + t_interval;
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function = kernel_timer_blink;

  add_timer(&mydata.timer);
}

ssize_t
kernel_timer_write(struct file *file, const char *gdata, size_t length, loff_t *off_what)
{
  long tmp = (long) gdata;

//  long kernel_timer_buff = 0;

//  if ( copy_from_user(&kernel_timer_buff, tmp, sizeof(long)) )
//    return -EFAULT;
  

  /* modify HZ to interval after I set timer interval at interval variable */
  mydata.count    = tmp & 0xFF;
  mydata.pat_fnd  = (tmp & 0xFF0000) >> 16;
  mydata.loc_fnd  = (tmp & 0xFF000000) >> 24;
  t_interval      = (unsigned long)((tmp & 0xFF00) >> 8) * HZ / 10;

  del_timer_sync(&mydata.timer);


  kernel_timer_device_set();

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
