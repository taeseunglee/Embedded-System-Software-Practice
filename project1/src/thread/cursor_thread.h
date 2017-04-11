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


struct cursor{
  int x, y;
};
// used as a pthread and print_cursor function argument
struct argu_mode_cursor {
  unsigned int *mode, *cursor_hide;
  struct cursor* cursor;
  unsigned char* mask;
  struct environment *env;
};


void* print_cursor(void *arguments);
