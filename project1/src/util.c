#include "util.h"
void kill_all_processes(struct environment *env)
{
  kill(env->pid_input, SIGINT);
  kill(env->pid_output, SIGINT);
  kill(env->pid_main, SIGINT);
}
/* TODO: move led_flicker and print_cursor to src/process/main_process.c */
/* In mode1, this is called main process by pthread.
 * LEDs come out alternately every second.
 */
void* led_flicker(void *arguments)
{
  struct argu_led_flick* argu = arguments;
  unsigned char led_data;
  int *led_flick = argu->led_flick;
  struct environment *env = argu->env;
  int i = 0;
  int led_fd = env->led_fd;
  unsigned int *cur_led = argu->cur_led,
               *time_second = argu->time_second;

  while (*led_flick && !quit) {
    *cur_led = 32;
    led_data = 32 | (*time_second); write(led_fd, &led_data, 1);
    i = 4;
    do { 
      usleep(245000);
      if (quit || !(*led_flick)) break;
    } while(--i);

    *cur_led = 16;
    led_data = 16 | (*time_second); write(led_fd, &led_data, 1);
    i = 4;
    do { 
      usleep(245000);
      if (quit || !(*led_flick)) break;
    } while(--i);
  }
  led_data = 128 | (*time_second); write(led_fd, &led_data, 1);

  pthread_exit(NULL);
}


/* For mode4 */
/* print cursor at DOT device */
void* print_cursor(void *arguments)
{
  struct argu_mode_cursor *argu = arguments;

  int * mode = argu->mode,
      * cursor_hide = argu->cursor_hide;
  struct cursor* cursor = argu->cursor;
  unsigned char *mask = argu->mask, res[10] = {0};

  struct environment *env = argu->env;
  int dot_fd = env->dot_fd;

  unsigned int size_mask = 10*sizeof(char);

  int i = 0;

  while ((*mode) == 3 && !quit) {
    if (*cursor_hide) {
      write(dot_fd, mask, size_mask);
    }
    else {
      i = 4;
      do {
        memcpy(res, mask, size_mask);
        res[cursor->y] ^= (0x80>>(cursor->x));
        write(dot_fd, res, size_mask);
        usleep(250000);
      } while(--i);
    }

    if ((*mode) != 3 || quit) break;

    // hide
    i = 4;
    do {
        write(dot_fd, res, size_mask);
        usleep(250000);
    } while(--i);

    usleep(1000000);
  }
  memset(mask, 0, size_mask);
  write(dot_fd, mask, size_mask);

  pthread_exit(NULL);
}

void quit_signal(int sig)
{
  quit = 1;
}

void device_clear(struct environment *env) {
  unsigned blank_data[50] = {0,};

  write(env->led_fd, blank_data, 1);
  write(env->fnd_fd, blank_data, 4);
  write(env->lcd_fd, blank_data, MAX_TEXT);
  write(env->dot_fd, blank_data, MAX_DOT);
}
