#include "cursor_thread.h"

/* In mode4, this is called to main process as a start routine of pthread for
 * print cursor at DOT device */
void* print_cursor(void *arguments)
{
  struct argu_mode_cursor *argu = arguments;

  unsigned int * mode = argu->mode,
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


