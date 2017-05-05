#ifndef __MODE_COUNTER__
#define __MODE_COUNTER__

#include "../process/process.h"

void mode_counter_global_init (struct environment * __env, int __msqid);
void mode_counter_init(void);
void mode_counter(message_buf rcv_buf);

#endif /* __MODE_COUNTER__ */
