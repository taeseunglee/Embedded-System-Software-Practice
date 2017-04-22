#ifndef __PROCESS__
#define __PROCESS__

/* files in lib */
#include <device.h>
#include <define.h>
#include <environment.h>
#include <message.h>


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// include headers for get input event
#include <fcntl.h>
#include <dirent.h>
#ifdef linux
#include <linux/input.h>
#else
// remove error message when I code in mac
struct input_event {
  int ev[1], value, code;
};
#endif


#include <termios.h>
#include <signal.h>

// include system headers
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/wait.h> // for waitpid
#include <sys/ioctl.h> // For pushing switch

// include headers for message queue(IPC)
#include <sys/ipc.h>
#include <sys/msg.h>

// Multi thread
#include <pthread.h>

#include <errno.h>

#endif /* __PROCESS__ */
