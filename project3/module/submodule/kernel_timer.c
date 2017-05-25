#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#define END_TIME 3

static int kernel_timer_usage = 0;

int kernel_timer_open(struct inode *, struct file *);
int kernel_timer_release(struct inode *, struct file *);
ssize_t kernel_timer_write(struct file *, const char *, size_t, loff_t *);

static struct file_operations kernel_timer_fops =
{
  .open = kernel_timer_open,
  .write = kernel_timer_write,
  .release = kernel_timer_release
};

static struct struct_mydata
{
  struct timer_list timer;
  int sec;
  int is_pause;
};

struct struct_mydata mydata;
struct struct_mydata end_data;

int kernel_timer_release(struct inode *minode, struct file *mfile) 
{
  printk("kernel_timer_release\n");
  kernel_timer_usage = 0;
  del_timer_sync(&mydata.timer);

  return 0;
}

int kernel_timer_open(struct inode *minode, struct file *mfile) 
{
  printk("kernel_timer_open\n");
  if (kernel_timer_usage != 0) 
    {
      return -EBUSY;
    }
  kernel_timer_usage = 1;
  init_timer(&(mydata.timer));

  return 0;
}

void
kernel_timer_blink(unsigned long timeout) 
{
  struct struct_mydata *p_data = (struct struct_mydata*)timeout;
  if (p_data->is_pause)
    return ;

  printk("kernel_timer_blink %d\n", p_data->sec);

  ++ p_data->sec;
  // the time is larger than 1 hour.
  if (!(p_data->sec < 3600))
    p_data->sec = 0;

  char dev_data[4];
  int sec = p_data;
  // TODO: Must modify  NULL.
  dev_data[0] = sec/600;
  dev_data[1] = (sec/60)%10;
  dev_data[2] = (sec%60)/10;
  dev_data[3] = sec%10;
  iom_fpga_fnd_fops.write(NULL, dev_data, 4, 0);


  // TODO: Modify mydata to p_data
  mydata.timer.expires = get_jiffies_64() + (1 * HZ);
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function = kernel_timer_blink;

  add_timer(&mydata.timer);
}

void
kernel_timer_end_blink(unsigned long timeout) 
{
  struct struct_mydata *p_data = (struct struct_mydata*)timeout;

  /* VOL- button released, just stop it */
  if (p_data->is_pause)
    return ;

  printk("kernel_timer_end_blink %d\n", p_data->sec);

  -- p_data->sec;

  /* Terminate an application and init FND */
  if (!p_data->sec)
    {
      char dev_data[4];
      memset(dev_data, 0, 4);
      // TODO: Must modify  NULL.
      iom_fpga_fnd_fops.write(NULL, dev_data, 4, 0);
      return ;
    }

  end_data.timer.expires = get_jiffies_64() + (1 * HZ);
  end_data.timer.data = (unsigned long)&end_data;
  end_data.timer.function = kernel_timer_end_blink;

  add_timer(&end_data.timer);
}


ssize_t kernel_timer_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
  const char *tmp = gdata;
  char kernel_timer_buff = 0;

  printk("write\n");
  // 1 byte
  if (copy_from_user(&kernel_timer_buff, tmp, 1))
    {
      return -EFAULT;
    }

  mydata.sec = kernel_timer_buff;

  printk("data  : %d \n",mydata.sec);

  del_timer_sync(&mydata.timer);

  mydata.timer.expires = jiffies + (1 * HZ);
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function	= kernel_timer_blink;

  add_timer(&mydata.timer);
  return 1;
}
