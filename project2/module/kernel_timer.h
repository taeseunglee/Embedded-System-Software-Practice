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
/* TODO: device.h를 만들어 MAX_DOT 등을 적는다 */
#define KTIMER_SET_FND _IOW(KERNEL_TIMER_MAJOR, 4, int)
#define KTIMER_SET_LED _IOW(KERNEL_TIMER_MAJOR, 1, int)
#define KTIMER_SET_DOT _IOW(KERNEL_TIMER_MAJOR, 10, int)
#define KTIMER_SET_LCD _IOW(KERNEL_TIMER_MAJOR, 32, int)


/* _IOR means that we're creating an ioctl command 
 * number for passing information from a user process
 * to the kernel module. 
 *
 * The first arguments, MAJOR_NUM, is the major device 
 * number we're using.
 *
 * The second argument is the number of the command 
 * (there could be several with different meanings).
 *
 * The third argument is the type we want to get from 
 * the process to the kernel.
 */



#endif /* __KERNEL_TIMER_H__ */
