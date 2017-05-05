#include "led_flicker_thread.h"

/* In mode1, this is called to main process as a start routine of pthread.
 * LEDs come out alternately every second.
 */
void* led_flicker(void *arguments)
{
  struct argu_led_flick* argu = arguments;
  struct environment *env = argu->env;
  int *led_flick = argu->led_flick;
  int led_fd = env->led_fd;
  unsigned int *cur_led = argu->cur_led,
               *time_second = argu->time_second;

  int i = 0;
  unsigned char led_data;

  while (*led_flick && !quit)
    {
      *cur_led = 32;
      led_data = 32 | (*time_second); write(led_fd, &led_data, 1);
      i = 4;
      do
        { 
          usleep(245000);
          if (quit || !(*led_flick)) break;
        } while(--i);

      *cur_led = 16;
      led_data = 16 | (*time_second); write(led_fd, &led_data, 1);
      i = 4;
      do
        { 
          usleep(245000);
          if (quit || !(*led_flick)) break;
        } while(--i);
    }
  led_data = 128 | (*time_second);
  write(led_fd, &led_data, 1);

  pthread_exit(NULL);
}

