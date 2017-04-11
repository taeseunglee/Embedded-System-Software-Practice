#include "util.h"
void device_clear(struct environment *env)
{
  unsigned blank_data[50] = {0,};

  write(env->led_fd, blank_data, 1);
  write(env->fnd_fd, blank_data, 4);
  write(env->lcd_fd, blank_data, MAX_TEXT);
  write(env->dot_fd, blank_data, MAX_DOT);
}
