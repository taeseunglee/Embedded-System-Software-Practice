#ifndef __MODE_CLOCK__
#define __MODE_CLOCK__

#include "../thread/led_flicker_thread.h"
#include "../message.h"

void mode_clock_global_init(struct environment *__env, int __msqid);
void mode_clock_init(void);
void mode_clock(message_buf rcv_buf);

#endif
