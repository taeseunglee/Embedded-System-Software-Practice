#ifndef __MODE_DRAW_BOARD__
#define __MODE_DRAW_BOARD__

#include "../lib/environment.h"
#include "../lib/message.h"
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

void mode_draw_board_global_init(struct environment *__env, int __msqid);
void mode_draw_board_init();
void mode_draw_board_exit();
void mode_draw_board(message_buf rcv_buf);


#endif /* __MODE_DRAW_BOARD__ */
