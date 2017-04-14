#include "../message.h"
#include "../environment.h"
#include "../process/process.h"

void
mode_clock_global_init (struct environment * __env, int __msqid);

static void mode_counter_board_init(void);
void mode_counter(message_buf rcv_buf);

