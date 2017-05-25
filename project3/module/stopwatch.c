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
#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/ioport.h>
#include <linux/version.h>
#include <linux/cdev.h>
#include <linux/sched.h>

#include "./submodule/fpga_fnd_driver.c"
#include "./submodule/kernel_timer.c"

#define DEV_NAME "/dev/stopwatch"

// TODO: Change "inter" to "stopwatch"

static int inter_major=242, inter_minor=0;
static int result;
static dev_t inter_dev;
static struct cdev inter_cdev;



static int inter_open(struct inode *, struct file *);
static int inter_release(struct inode *, struct file *);
static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos);

irqreturn_t start_timer(int irq, void* dev_id); /* HOME button */
irqreturn_t pause_timer(int irq, void* dev_id); /* BACK button */
irqreturn_t reset_timer(int irq, void* dev_id); /* VOL+ */
irqreturn_t end_timer(int irq, void* dev_id); /* VOL- */

static int inter_usage=0;
int interruptCount=0;

DECLARE_WAIT_QUEUE_HEAD(wq_write);

static struct file_operations inter_fops =
{
  .open = inter_open,
  .write = inter_write,
  .release = inter_release,
};

irqreturn_t
start_timer(int irq, void* dev_id)
{
  printk(KERN_ALERT "[LOG] start_timer\n");
  mydata.is_pause = 0;

  if (timer_set)
    {
      del_timer_sync(&mydata.timer);
      timer_set = 0;
    }

  mydata.timer.expires = get_jiffies_64() + (1 * HZ);
  mydata.timer.data = (unsigned long)&mydata;
  mydata.timer.function	= kernel_timer_blink;

  add_timer(&mydata.timer);
  timer_set = 1;

  return IRQ_HANDLED;
}

irqreturn_t
pause_timer(int irq, void* dev_id)
{
  printk(KERN_ALERT "[LOG] pause_timer\n");

  mydata.is_pause = 1;

  return IRQ_HANDLED;
}

irqreturn_t
reset_timer(int irq, void* dev_id)
{
  printk(KERN_ALERT "[LOG] reset_timer\n");
  /* Reset time */
  outw((unsigned short int)0, (unsigned int)iom_fpga_fnd_addr);

  mydata.sec = 0;
  mydata.is_pause = 1;

  return IRQ_HANDLED;
}

irqreturn_t
end_timer(int irq, void* dev_id)
{
  static int is_pressed = 0;

  printk(KERN_ALERT "[LOG] end_timer\n");

  // init fnd
  if (!is_pressed)
    {
      is_pressed = 1;
      // wait end_data if it exists
      if (end_timer_set)
        del_timer_sync(&end_data.timer);

      end_timer_set = 1;
      end_data.sec = 0;
      end_data.is_pause = 0;
      end_data.timer.expires = get_jiffies_64() + (1 * HZ);
      end_data.timer.data = (unsigned long)&end_data;
      end_data.timer.function = kernel_timer_blink;
      add_timer(&end_data.timer);

      return IRQ_HANDLED;
    }
  else
    {
      is_pressed = 0;

      end_data.is_pause = 1;
      if (end_timer_set)
        {
          del_timer_sync(&end_data.timer);
          end_timer_set = 0;
        }
  
      return IRQ_HANDLED;
    }
}

static int inter_open(struct inode *minode, struct file *mfile){
    int ret;
    int irq;

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
    ret=request_irq(irq, reset_timer, IRQF_TRIGGER_FALLING, "volup", 0); // TODO: check return

    // VOL- Pressed
    gpio_direction_input(IMX_GPIO_NR(5,14));
    irq = gpio_to_irq(IMX_GPIO_NR(5,14));
    printk(KERN_ALERT "IRQ Number : %d\n",irq);
    ret=request_irq(irq, end_timer, IRQF_TRIGGER_FALLING | IRQF_TRIGGER_RISING, "voldown", 0);

    // Init timer
    init_timer(&mydata.timer);
    init_timer(&end_data.timer);
    timer_set = 0;
    end_timer_set = 0;
    mydata.sec = 0;
    mydata.is_end = 0;
    end_data.is_end = 1;

    return 0;
}

static int inter_release(struct inode *minode, struct file *mfile)
{
  if (timer_set)
    {
      del_timer_sync(&mydata.timer);
      timer_set = 0;
    }
  if (end_timer_set)
    {
      del_timer_sync(&end_data.timer);
      end_timer_set = 0;
    }

  free_irq(gpio_to_irq(IMX_GPIO_NR(1, 11)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(1, 12)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(2, 15)), NULL);
  free_irq(gpio_to_irq(IMX_GPIO_NR(5, 14)), NULL);

  printk(KERN_ALERT "Release Module\n");
  return 0;
}

static int inter_write(struct file *filp, const char *buf, size_t count, loff_t *f_pos ){
    if(interruptCount==0)
      {
        printk("sleep on\n");
        interruptible_sleep_on(&wq_write);
      }

    printk("write\n");
    return 0;
}

static int inter_register_cdev(void)
{
  int error;
  if(inter_major) 
    {
      inter_dev = MKDEV(inter_major, inter_minor);
      error = register_chrdev_region(inter_dev,1,"stopwatch");
    }else
      {
        error = alloc_chrdev_region(&inter_dev,inter_minor,1,"stopwatch");
        inter_major = MAJOR(inter_dev);
      }
    if(error<0) 
      {
        printk(KERN_WARNING "stopwatch: can't get major %d\n", inter_major);
        return result;
      }
    printk(KERN_ALERT "major number = %d\n", inter_major);
    cdev_init(&inter_cdev, &inter_fops);
    inter_cdev.owner = THIS_MODULE;
    inter_cdev.ops = &inter_fops;
    error = cdev_add(&inter_cdev, inter_dev, 1);
    if(error)
      {
        printk(KERN_NOTICE "inter Register Error %d\n", error);
      }
    return 0;
}

static int __init inter_init(void)
{
  int result;
  if ((result = inter_register_cdev()) < 0 )
    return result;
  iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);

  printk(KERN_ALERT "Init Module Success \n");
  printk(KERN_ALERT "Device : %s, Major Num : 242\n", DEV_NAME);
  return 0;
}

static void __exit inter_exit(void)
{
  cdev_del(&inter_cdev);
  iounmap(iom_fpga_fnd_addr);
  unregister_chrdev_region(inter_dev, 1);
  printk(KERN_ALERT "Remove Module Success \n");
}

module_init(inter_init);
module_exit(inter_exit);
MODULE_LICENSE("GPL");
