#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>
#include "../lib/device.h"


struct __mode5_flag
  {
    unsigned int mode_time_goes: 1; // for mode 1
    unsigned int mode_4th_of_base10: 1; // for mode 2, print 4th digit while the base of counter is 10
  } mode5_flag;


struct environment {
  pid_t pid_input, pid_output, pid_main;
  int ev_fd, fnd_fd, led_fd,
      push_switch_fd, dot_fd,
      lcd_fd;
  int msg_key;

  /* For Mode5 */
  clock_t begin;
  struct __mode5_flag mode5_flag;
};

// TODO: save quit in env
extern unsigned int quit;


void construct_environment(struct environment** env);
void destruct_environment(struct environment* env);
void kill_all_processes(struct environment *env);
void quit_signal(int sig);


#endif
