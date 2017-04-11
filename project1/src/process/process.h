#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "../../lib/device.h"
#include "../../lib/define.h"
#include "../../src/environment.h"
#include "../util.h"

// For fpga dot font
// #include "../../lib/fpga_dot_font.h"


#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// include headers for get input event
#include <errno.h>
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


// usage: MSGSND_OR_DIE(msqid, &snd_buf, msgsz, IPC_NOWAIT);
#define MSGSND_OR_DIE(...) \
  if (msgsnd(__VA_ARGS__) < 0) { \
    perror("msgsnd"); \
    kill_all_processes(env); \
  }

#endif
