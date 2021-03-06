/* FPGA Dot Matrix Ioremap Control
FILE : fpga_dot_driver.c 
AUTH : largest@huins.com*/

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

#include "./fpga_dot_font.h"

#define IOM_FPGA_DOT_ADDRESS 0x08000210 // pysical address


//Global variable
static unsigned char *iom_fpga_dot_addr;

// define functions...
ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
void iom_fpga_dot_init(void);
void iom_fpga_dot_exit(void);

// define file_operations structure 
struct file_operations iom_fpga_dot_fops =
{
  .owner  = THIS_MODULE,
  .write  = iom_fpga_dot_write,	
};

// when write to fpga_dot device  ,call this function
ssize_t iom_fpga_dot_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
  int i;

  unsigned char value[10];
  unsigned short int _s_value;
  const char *tmp = gdata;

  memcpy((void __force *)value, tmp, length);

  for(i=0;i<length;i++)
    {
      _s_value = value[i] & 0x7F;
      outw(_s_value, (unsigned int)iom_fpga_dot_addr+i*2);
    }

  return length;
}


void iom_fpga_dot_init(void)
{
  iom_fpga_dot_addr = ioremap(IOM_FPGA_DOT_ADDRESS, 0x10);
}

void iom_fpga_dot_exit(void) 
{
  iounmap(iom_fpga_dot_addr);
}

