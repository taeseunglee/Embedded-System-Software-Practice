#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/platform_device.h>
#include <linux/kernel.h>
#include <linux/uaccess.h>

#define END_TIME 3

static int kernel_timer_usage = 0;
wait_queue_head_t wq_write;
int timer_set;
int end_timer_set;

int kernel_timer_open(struct inode *, struct file *);
int kernel_timer_release(struct inode *, struct file *);
ssize_t kernel_timer_write(struct file *, const char *, size_t, loff_t *);

struct struct_mydata
{
  struct timer_list timer;
  int sec;
  int is_pause;
  int is_end;
};

struct struct_mydata mydata;
struct struct_mydata end_data;

int kernel_timer_release(struct inode *minode, struct file *mfile) 
{
  printk("kernel_timer_release\n");
  kernel_timer_usage = 0;
  if (timer_set)
    {
      del_timer_sync(&mydata.timer);
      timer_set = 0;
    }

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
  init_timer(&(end_data.timer));

  return 0;
}

void
kernel_timer_blink(unsigned long timeout) 
{
  struct struct_mydata *p_data = (struct struct_mydata*)timeout;
  char dev_data[4];

  if (p_data->is_pause)
  {
    p_data->is_pause = 0;
    timer_set = 0;
    return ;
  }

  printk(KERN_ALERT "kernel_timer_blink\n");
  // end timer created by vol-
  if (p_data->is_end)
    {
      if (p_data->is_pause)
        {
          end_timer_set = 0;
          return ;
        }
      printk("kernel_timer_blink(end) %d\n", p_data->sec);
      ++ p_data->sec;

      // terminate an application
      if (p_data->sec == END_TIME)
        {
          if (timer_set)
            {
              del_timer_sync(&mydata.timer);
              timer_set = 0;
            }
          if (end_timer_set)
            end_timer_set = 0;
          wake_up_interruptible(&wq_write);

          outw((unsigned short int)0,
               (unsigned int)iom_fpga_fnd_addr);
          return ;
        }
    }
  else // timer
    {
      int sec;
      unsigned int short value_short;

      ++ p_data->sec;
      printk("kernel_timer_blink %d\n", p_data->sec);

      // the time is larger than 1 hour.
      if (!(p_data->sec < 3600))
        p_data->sec = 0;

      sec = p_data->sec;
      // TODO: Must modify  NULL.
      dev_data[0] = sec/600;
      dev_data[1] = (sec/60)%10;
      dev_data[2] = (sec%60)/10;
      dev_data[3] = sec%10;

      value_short = (dev_data[0] << 12) | (dev_data[1] << 8) | (dev_data[2] << 4) | dev_data[3];

      outw(value_short, (unsigned int)iom_fpga_fnd_addr);	
    }

  p_data->timer.expires = get_jiffies_64() + (1 * HZ);
  p_data->timer.data = (unsigned long)p_data;
  p_data->timer.function = kernel_timer_blink;

  add_timer(&p_data->timer);
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


  mydata.timer.expires = jiffies + (1 * HZ);
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function	= kernel_timer_blink;

  add_timer(&mydata.timer);
  return 1;
}
