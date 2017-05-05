#ifndef __MODE_DRAW_BOARD__
#define __MODE_DRAW_BOARD__

#include "../lib/environment.h"
#include "../lib/message.h"

void mode_draw_board_global_init(struct environment *__env, int __msqid);
void mode_draw_board_init();
void mode_draw_board(message_buf rcv_buf);


#endif /* __MODE_DRAW_BOARD__ */
