#include <linux/string.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

/* ioctl numbers */
#include "./kernel_timer.h"

#include "./submodule/fpga_dot_driver.c"
#include "./submodule/fpga_fnd_driver.c"
#include "./submodule/fpga_led_driver.c"
#include "./submodule/fpga_text_lcd_driver.c"



/* shift right */
#define __SHIFT_TEXT_RIGHT(str) \
  do { \
    int idx = 15; \
    do { \
      str[idx] = str[idx-1]; \
    } while (--idx); \
    str[0] = 0;\
  } while (0);

/* shift left */
#define __SHIFT_TEXT_LEFT(str) \
  do { \
    int idx = 0; \
    do { \
      str[idx] = str[idx+1]; \
      ++idx; \
    } while(idx < 15); \
    str[15] = 0; \
  } while(0);

/* Direction value
 * RIGHT = 0, LEFT = 1
 * */
#define SHIFT_TEXT(str, direction) \
  { \
    if (direction) { __SHIFT_TEXT_LEFT(str); } \
    else { __SHIFT_TEXT_RIGHT(str); } \
  }
 

static int remained_stn_cnt;
static int remained_name_cnt;
static int direction_stn;
static int direction_name;

int kernel_timer_open(struct inode *, struct file *);
int kernel_timer_release(struct inode *, struct file *);
ssize_t kernel_timer_write(struct file *, const char *, size_t, loff_t *);
long kernel_timer_ioctl(struct file *, unsigned int, unsigned long);
void kernel_timer_device_set(struct file * file);

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
  struct file *file;
  int count;
  int pat_fnd;
  int loc_fnd;
};

/* stn : student number */
/* direction: 0 - right, 1 - left */
static struct struct_mydata mydata;
static unsigned long t_interval;
static char stn[16];
static char name[16];
static int len_stn;
static int len_name;

#define STUDENT_NUMBER "20141570"
#define NAME "Taeseung Lee"

void
kernel_timer_device_set(struct file * file)
{
  char dev_data[33];


  /* TODO: Error check */
  /** set devices **/

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
  memcpy(dev_data, stn, 16*sizeof(char));
  memcpy(dev_data+16, name, 16*sizeof(char));
  iom_fpga_text_lcd_fops.write(file, dev_data, 32, 0);


  ++ mydata.pat_fnd;
  if (mydata.pat_fnd > 8)
    {
      mydata.pat_fnd = 1;
      ++ mydata.loc_fnd;
      mydata.loc_fnd &= 0x03;
    }

  if (!remained_stn_cnt)
    {
      remained_stn_cnt = 16 - len_stn;
      direction_stn ^= 1;
    }

  SHIFT_TEXT(stn, direction_stn);


  if (!remained_name_cnt)
    {
      remained_name_cnt = 16 - len_name;
      direction_name ^= 1;
    }

  SHIFT_TEXT(name, direction_name);

  -- remained_stn_cnt;
  -- remained_name_cnt;
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
  if (mydata.count --)
    {
      kernel_timer_device_set(mydata.file);

      mydata.timer.expires = get_jiffies_64() + t_interval;
      mydata.timer.data = (unsigned long)&mydata;
      mydata.timer.function = kernel_timer_blink;

      add_timer(&mydata.timer);
    }
  else // clear the board
    {
      char dev_data[32];
      memset(dev_data, 0, 32);

      iom_fpga_fnd_fops.write(mydata.file, dev_data, 4, 0);
      iom_led_fops.write(mydata.file, dev_data, 1, 0);
      iom_fpga_dot_fops.write(mydata.file, dev_data, 10, 0);
      iom_fpga_text_lcd_fops.write(mydata.file, dev_data, 32, 0);

      printk("Kernel timer ends\n");
    }
}

ssize_t
kernel_timer_write(struct file *file, const char *gdata, size_t length, loff_t *off_what)
{
  long tmp = (long) gdata;

  mydata.file     = file;
  mydata.count    = tmp & 0xFF;
  mydata.pat_fnd  = (tmp & 0xFF0000) >> 16;
  mydata.loc_fnd  = (tmp & 0xFF000000) >> 24;
  t_interval      = (unsigned long)((tmp & 0xFF00) >> 8) * HZ / 10;

  del_timer_sync(&mydata.timer);


  remained_stn_cnt = 16 - len_stn;
  remained_name_cnt = 16 - len_name;
  direction_stn = direction_name = 0;

  strncpy(stn, STUDENT_NUMBER, 16);
  strncpy(name, NAME, 16);


  mydata.timer.expires  = jiffies_64 + t_interval;
  mydata.timer.data     = (unsigned long)&mydata;
  mydata.timer.function	= kernel_timer_blink;

  add_timer(&mydata.timer);

  return 1;
}

/* ioctl_num : The number of the ioctl */
/* ioctl_param : The parameter to it */
long
kernel_timer_ioctl(struct file *file, unsigned int ioctl_num, unsigned long ioctl_param)
{
  /* Switch according to the ioctl called */
  switch (ioctl_num)
    {
    case KTIMER_START:
      kernel_timer_write(file, (char *) ioctl_param, 0, 0);
      break;
    }

  return 0;
}

int __init 
kernel_timer_init(void)
{
  int result;

  struct class *kernel_timer_dev_class = NULL;
  struct device*kernel_timer_dev = NULL;

  printk("kernel_timer_init\n");

  len_stn = strlen(STUDENT_NUMBER);
  len_name = strlen(NAME);

  if (len_stn > 16)
    {
      printk("The Student Number is too long(length: %d)\n", len_stn);
      return -1;
    }

  if (len_name > 16)
    {
      printk("The name is too long(length: %d)\n", len_name);
      return -1;
    }

  iom_fpga_fnd_init();
  iom_fpga_dot_init();
  iom_led_init();
  iom_fpga_text_lcd_init();
  

  result = register_chrdev(KERNEL_TIMER_MAJOR, KERNEL_TIMER_NAME, &kernel_timer_fops);
  if(result <0)
    {
      printk( "Kernel_timer_init error %d\n",result);
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

  iom_fpga_fnd_exit();
  iom_fpga_dot_exit();
  iom_led_exit();
  iom_fpga_text_lcd_exit();

  unregister_chrdev(KERNEL_TIMER_MAJOR, KERNEL_TIMER_NAME);
}

module_init( kernel_timer_init);
module_exit( kernel_timer_exit);

MODULE_LICENSE ("GPL");
MODULE_AUTHOR ("author");
