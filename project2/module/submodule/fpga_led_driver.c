/* FPGA LED Ioremap Control
FILE : fpga_led_driver.c 
AUTH : largest@huins.com */

#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/slab.h>
#include <linux/platform_device.h>
#include <linux/delay.h>

#include <asm/io.h>
#include <asm/uaccess.h>
#include <linux/kernel.h>
#include <linux/ioport.h>
#include <linux/module.h>
#include <linux/fs.h>
#include <linux/init.h>
#include <linux/version.h>


#define IOM_LED_ADDRESS 0x08000016 // pysical address


//Global variable
static unsigned char *iom_fpga_led_addr;

// define functions...
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
void iom_led_init(void);
void iom_led_exit(void);

// define file_operations structure 
struct file_operations iom_led_fops =
{
  .owner    = THIS_MODULE,
  .write    = iom_led_write,	
};

// when write to led device  ,call this function
ssize_t iom_led_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
  unsigned char value;
  unsigned short _s_value;
  const char *tmp = gdata;


  memcpy(&value, tmp, length);

  _s_value = (unsigned short)value;
  outw(_s_value, (unsigned int)iom_fpga_led_addr);

  return length;
}

void iom_led_init(void)
{
  iom_fpga_led_addr = ioremap(IOM_LED_ADDRESS, 0x1);
}

void iom_led_exit(void) 
{
  iounmap(iom_fpga_led_addr);
}
