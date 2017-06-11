/* FPGA TEXT_LCD Ioremap Control
FILE : fpga_text_lcd_driver.c 
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


#define IOM_FPGA_TEXT_LCD_ADDRESS 0x08000090 // pysical address - 32 Byte (16 * 2)


//Global variable
static unsigned char *iom_fpga_text_lcd_addr;

// define functions...
ssize_t iom_fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what);
void iom_fpga_text_lcd_init(void);
void iom_fpga_text_lcd_exit(void);

// define file_operations structure 
struct file_operations iom_fpga_text_lcd_fops =
{
  .owner  = THIS_MODULE,
  .write  = iom_fpga_text_lcd_write,	
};

// when write to fpga_text_lcd device  ,call this function
ssize_t iom_fpga_text_lcd_write(struct file *inode, const char *gdata, size_t length, loff_t *off_what) 
{
  int i;

  unsigned short int _s_value = 0;
  unsigned char value[33];
  const char *tmp = gdata;

  memcpy((void __force *)value, tmp, length);

  value[length]=0;
  for(i=0;i<length;i++)
    {
      _s_value = (value[i] & 0xFF) << 8 | (value[i + 1] & 0xFF);
      outw(_s_value, (unsigned int)iom_fpga_text_lcd_addr+i);
      i++;
    }

  return length;
}


void iom_fpga_text_lcd_init(void)
{
  iom_fpga_text_lcd_addr = ioremap(IOM_FPGA_TEXT_LCD_ADDRESS, 0x32);
}

void iom_fpga_text_lcd_exit(void) 
{
  iounmap(iom_fpga_text_lcd_addr);
}
