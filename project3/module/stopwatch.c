#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/interrupt.h>
#include <asm/irq.h>
#include <mach/gpio.h>
#include <linux/platform_device.h>
#include <asm/gpio.h>
#include <linux/wait.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/uaccess.h>
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/sched.h>

#define DEV_NAME "/dev/stopwatch"
#define END_SECOND 3
#define IOM_FND_ADDRESS 0x08000004 // FND pysical address

unsigned char *iom_fpga_fnd_addr;

static int stopwatch_major=245, stopwatch_minor=0;
static dev_t stopwatch_dev;
static struct cdev stopwatch_cdev;

wait_queue_head_t wq_write;

struct timer_info
{
  struct timer_list timer;
  int sec;
  int is_pause;
  int is_end;
  int is_set;
};

/* Manage timer list variable */
struct timer_info main_timer_info;
struct timer_info end_timer_info;

static int stopwatch_usage=0;
int interruptCount=0;

DECLARE_WAIT_QUEUE_HEAD(wq_write);

static int stopwatch_open(struct inode *, struct file *);
static int stopwatch_release(struct inode *, struct file *);
static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

/* Interrupt Handler */
irqreturn_t start_timer(int irq, void* dev_id); /* HOME button */
irqreturn_t pause_timer(int irq, void* dev_id); /* BACK button */
irqreturn_t reset_timer(int irq, void* dev_id); /* VOL+ */
irqreturn_t end_timer(int irq, void* dev_id); /* VOL- */


static struct file_operations stopwatch_fops =
{
  .open = stopwatch_open,
  .write = stopwatch_write,
  .release = stopwatch_release,
};



void
kernel_timer_blink(unsigned long timeout) 
{
 struct timer_info *p_data = (struct timer_info*)timeout;
  char dev_data[4];

  if (p_data->is_pause)
  {
    p_data->is_pause = 0;
    p_data->is_set = 0;
    return ;
  }

  // end timer created by vol-
  if (p_data->is_end)
    {
      printk("kernel_timer_blink(with end_timer)(second) %d\n", p_data->sec);
      ++ p_data->sec;

      // terminate an application
      if (p_data->sec == END_SECOND)
        {
          if (main_timer_info.is_set)
            {
              del_timer(&main_timer_info.timer);
              main_timer_info.is_set = 0;
            }
          if (end_timer_info.is_set)
            end_timer_info.is_set = 0;
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
      // the time is larger than 1 hour.
      if (!(p_data->sec < 3600))
        p_data->sec = 0;
      printk("kernel_timer_blink current time(second): %d\n", p_data->sec);

      /* Set time at fnd */
      sec = p_data->sec;
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

  irqreturn_t
start_timer(int irq, void* dev_id)
{
  printk(KERN_ALERT "[LOG] start_timer\n");

  

  main_timer_info.is_pause = 0;

  // If you push start key but timer is already executed
  if (main_timer_info.is_set)
    return IRQ_HANDLED;

  main_timer_info.timer.expires = get_jiffies_64() + (1 * HZ);
  main_timer_info.timer.data = (unsigned long)&main_timer_info;
  main_timer_info.timer.function	= kernel_timer_blink;

  add_timer(&main_timer_info.timer);
  main_timer_info.is_set = 1;

  return IRQ_HANDLED;
}

  irqreturn_t
pause_timer(int irq, void* dev_id)
{
  printk(KERN_ALERT "[LOG] pause_timer\n");

  main_timer_info.is_pause = 1;

  return IRQ_HANDLED;
}

  irqreturn_t
reset_timer(int irq, void* dev_id)
{
  printk(KERN_ALERT "[LOG] reset_timer\n");
  /* Reset time */
  outw((unsigned short int)0, (unsigned int)iom_fpga_fnd_addr);

  main_timer_info.sec = 0;
  main_timer_info.is_pause = 1;

  return IRQ_HANDLED;
}

  irqreturn_t
end_timer(int irq, void* dev_id)
{
  static int is_pressed = 0;
  printk(KERN_ALERT "[LOG] end_timer\n");


  if (!is_pressed)
  {
    is_pressed = 1;
    // wait end_timer_info if it exists
    if (end_timer_info.is_set)
      del_timer(&end_timer_info.timer);

    end_timer_info.sec            = 0;
    end_timer_info.is_set         = 1;
    end_timer_info.is_pause       = 0;
    end_timer_info.timer.expires  = get_jiffies_64() + (1 * HZ);
    end_timer_info.timer.data     = (unsigned long) &end_timer_info;
    end_timer_info.timer.function = kernel_timer_blink;
    add_timer(&end_timer_info.timer);

    return IRQ_HANDLED;
  }
  else
  {
    is_pressed = 0;

    end_timer_info.is_pause = 1;
    if (end_timer_info.is_set)
    {
      del_timer(&end_timer_info.timer);
      end_timer_info.is_set = 0;
    }

    return IRQ_HANDLED;
  }
}

static int stopwatch_open(struct inode *minode, struct file *mfile)
{
  int ret;
  int irq;


  if (stopwatch_usage)
    return -EBUSY;

  stopwatch_usage = 1;

  printk(KERN_ALERT "Open Module\n");

  // HOME
  gpio_direction_input(IMX_GPIO_NR(1,11));
  irq = gpio_to_irq(IMX_GPIO_NR(1,11));
  printk(KERN_ALERT "IRQ Number : %d\n", irq);
  ret=request_irq(irq, start_timer, IRQF_TRIGGER_FALLING, "home", 0);

  // BACK
  gpio_direction_input(IMX_GPIO_NR(1, 12));
  irq = gpio_to_irq(IMX_GPIO_NR(1, 12));
  printk(KERN_ALERT "IRQ Number : %d\n",irq);
  ret=request_irq(irq, pause_timer, IRQF_TRIGGER_FALLING, "back", 0);

  // VOL+
  gpio_direction_input(IMX_GPIO_NR(2,15));
  irq = gpio_to_irq(IMX_GPIO_NR(2,15));
  printk(KERN_ALERT "IRQ Number : %d\n",irq);
  ret=request_irq(irq, reset_timer, IRQF_TRIGGER_FALLING, "volup", 0);

  // VOL- Pressed
  gpio_direction_input(IMX_GPIO_NR(5,14));
  irq = gpio_to_irq(IMX_GPIO_NR(5,14));
  printk(KERN_ALERT "IRQ Number : %d\n",irq);
  ret=request_irq(irq, end_timer, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "voldown", 0);

  // Init timer
  init_timer(&main_timer_info.timer);
  init_timer(&end_timer_info.timer);

  main_timer_info.sec = 0;
  main_timer_info.is_set = 0;
  main_timer_info.is_end = 0;

  end_timer_info.is_set = 0;
  end_timer_info.is_end = 1;

  return 0;
}

static int stopwatch_release(struct inode *minode, struct file *mfile)
{
  if (main_timer_info.is_set)
  {
    del_timer(&main_timer_info.timer);
    main_timer_info.is_set = 0;
  }
  if (end_timer_info.is_set)
  {
    del_timer(&end_timer_info.timer);
    end_timer_info.is_set = 0;
  }

  stopwatch_usage = 0;

  free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

  printk(KERN_ALERT "Release Module\n");
  return 0;
}

static int stopwatch_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos )
{
  if(interruptCount==0)
  {
    printk("sleep on\n");
    interruptible_sleep_on(&wq_write);
  }

  printk("write\n");
  return 0;
}

static int stopwatch_register_cdev(void)
{
  int error;
  if(stopwatch_major)
  {
    stopwatch_dev = MKDEV(stopwatch_major, stopwatch_minor);
    error = register_chrdev_region(stopwatch_dev,1,"stopwatch");
  }else
  {
    error = alloc_chrdev_region(&stopwatch_dev,stopwatch_minor,1,"stopwatch");
    stopwatch_major = MAJOR(stopwatch_dev);
  }
  if(error<0) 
  {
    printk(KERN_WARNING "stopwatch: can't get major %d\n", stopwatch_major);
    return error;
  }
  printk(KERN_ALERT "major number = %d\n", stopwatch_major);
  cdev_init(&stopwatch_cdev, &stopwatch_fops);
  stopwatch_cdev.owner = THIS_MODULE;
  stopwatch_cdev.ops = &stopwatch_fops;
  error = cdev_add(&stopwatch_cdev, stopwatch_dev, 1);
  if(error)
  {
    printk(KERN_NOTICE "stopwatch Register Error %d\n", error);
  }
  return 0;
}


/****************************************************************************/
static int __init stopwatch_init(void)
{
  int result;
  if ((result = stopwatch_register_cdev()) < 0 )
    return result;
  iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);

  printk(KERN_ALERT "Init Module Success \n");
  printk(KERN_ALERT "Device : %s, Major Num : %d\n", DEV_NAME, stopwatch_major);
  return 0;
}

static void __exit stopwatch_exit(void)
{
  cdev_del(&stopwatch_cdev);
  iounmap(iom_fpga_fnd_addr);
  unregister_chrdev_region(stopwatch_dev, 1);
  printk(KERN_ALERT "Remove Module Success \n");
}

module_init(stopwatch_init);
module_exit(stopwatch_exit);
MODULE_LICENSE("GPL");
