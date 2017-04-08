#ifndef __UTILS_H__
#define __UTILS_H__

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

#define SHIFT_TEXT(text) \
  do { \
	int t=0; \
	while (t<LIMIT_TEXT) { text[t] = text[t+1]; t++; } \
  } while(0);

#define MSG_SND(device, msqid, snd_buf, msgsz, type) \
  do { \
    snd_buf.mtext[0] = device; \
    snd_buf.mtype = type; \
    if (msgsnd(msqid, &snd_buf, msgsz, IPC_NOWAIT) < 0) { \
      printf("device num: %d, errno: %d, type: %d, msgsz: %zu\n", device, errno, type, msgsz); \
      perror("msgsnd"); \
      kill_all_processes(env); \
    } \
  } while(0);

struct cursor{
  int x, y;
};

// used as a pthread and led_filcker function argument
struct argu_led_flick {
  int *led_flick;
  unsigned int *cur_led;
  unsigned int *time_second;
  struct environment *env;
};

// used as a pthread and print_cursor function argument
struct argu_mode_cursor {
  unsigned int *mode, *cursor_hide;
  struct cursor* cursor;
  unsigned char* mask;
  struct environment *env;
};

extern unsigned int quit;

void  kill_all_processes(struct environment *env);
void* led_flicker(void *arguments);
void* print_cursor(void *arguments);
void  quit_signal(int sig);
void  device_clear(struct environment *env);

#endif
