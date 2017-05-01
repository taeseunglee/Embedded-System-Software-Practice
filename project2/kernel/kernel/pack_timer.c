#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <asm/uaccess.h>

asmlinkage long sys_pack_timer(long interval, long number, long startup_option)
{
  int res = 0, tmp;
  int i;
  int bound[] = {10000, 1000, 100, 10, 1};
  int init_loc_fnd, init_val_fnd;
  

  /* 1bit(the initial location of fnd)
     1bit(the initial value of fnd )
     1bit(time interval)
     1bit(the number of tic tok repititions)
  */

  /* format check */
  for (i = 5; i; --i)
    {
      tmp = startup_option % bound[i-1];
      if (tmp)
        {
          init_val_fnd = tmp / bound[i];
          init_loc_fnd = i - 1;
          break;
        }
    }

  res = (init_loc_fnd << 24)
    | (init_val_fnd << 16)
    | ((interval << 8)&0xFF00)
    | (number&0xFF);

  return 0;
}

int pack_timer_init( void )
{
  return 0;
}
void pack_timer_exit( void ) { }

EXPORT_SYMBOL( sys_pack_timer );

module_init ( pack_timer_init );
module_exit ( pack_timer_exit );
MODULE_LICENSE("Dual BSD/GPL");
