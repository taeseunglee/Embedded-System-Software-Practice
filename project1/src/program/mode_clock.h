#ifndef __MODE_CLOCK__
#define __MODE_CLOCK__

#include "../lib/message.h"
#include "../lib/environment.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <string.h>

// used as a pthread and led_filcker function argument
struct argu_led_flick {
  int *led_flick;
  unsigned int *cur_led;
  unsigned int *time_second;
  struct environment *env;
};


void* led_flicker(void *arguments);
void mode_clock_global_init(struct environment *__env, int __msqid);
void mode_clock_init(void);
void mode_clock_exit(void);
void mode_clock(message_buf rcv_buf);

#endif /* __MODE_CLOCK__ */
