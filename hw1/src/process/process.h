#ifndef __PROCESS_H__
#define __PROCESS_H__

#include "lib/error_check.h"
#include "lib/device.h"
#include "lib/environment.h"
#include "lib/define.h"
#include "src/util.h"

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

// include headers for get input event
#include <errno.h>
#include <fcntl.h>
#include <dirent.h>
#include <linux/input.h>
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

// For fpga dot font
#include "lib/fpga_dot_font.h"

#endif
