#include "../src/environment.h"
#include "../lib/define.h"
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

extern unsigned int quit;

void* led_flicker(void *arguments);
