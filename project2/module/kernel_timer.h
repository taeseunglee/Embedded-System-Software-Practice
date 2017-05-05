#ifndef __KERNEL_TIMER_H__
#define __KERNEL_TIMER_H__

#include <linux/ioctl.h>

/* The major device number. We can't rely on dynamic 
 * registration any more, because ioctls need to know 
 * it. */
#define KERNEL_TIMER_MAJOR 242
#define KERNEL_TIMER_MINOR 0
/* The name of the device file */
#define KERNEL_TIMER_NAME "dev_driver" // TODO: change this to "kernel_timer"

/* _IO, _IOW, _IOR, _IORW are helper macros to create a unique ioctl identifier
and add the required R/W needed features (direction). */
#define KTIMER_START  _IOW(KERNEL_TIMER_MAJOR, 1, char*)

#endif /* __KERNEL_TIMER_H__ */
