#ifndef __ENVIRONMENT_H__
#define __ENVIRONMENT_H__

#include <unistd.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <signal.h>
#include "../lib/device.h"
#include "../lib/error_check.h"
#include "../src/util.h"

struct environment {
  pid_t pid_input, pid_output, pid_main;
  int ev_fd, fnd_fd, led_fd, push_switch_fd,
      dot_fd, lcd_fd;
};

struct environment*
construct_environment(struct environment** env);

void
destruct_environment(struct environment* env);

#endif
