/* FPGA FND Ioremap Control
FILE : fpga_fpga_driver.c 
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


#define IOM_FND_ADDRESS 0x08000004 // pysical address

//Global variable
static unsigned char *iom_fpga_fnd_addr;

// define functions...
ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
void iom_fpga_fnd_init(void);
void iom_fpga_fnd_exit(void);

// define file_operations structure 
struct file_operations iom_fpga_fnd_fops =
{
  .owner  = THIS_MODULE,
  .write  = iom_fpga_fnd_write,	
};


// when write to fnd device, call this function
ssize_t iom_fpga_fnd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
  unsigned char value[4];
  unsigned short int value_short = 0;
  const char *tmp = gdata;

  memcpy((void __force *)value, tmp, length);

  value_short = (value[0] << 12) | (value[1] << 8) | (value[2] << 4) | value[3];
  outw(value_short, (unsigned int)iom_fpga_fnd_addr);	    

  return length;
}

void iom_fpga_fnd_init(void)
{
  iom_fpga_fnd_addr = ioremap(IOM_FND_ADDRESS, 0x4);
}

void iom_fpga_fnd_exit(void) 
{
  iounmap(iom_fpga_fnd_addr);
}
