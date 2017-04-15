#ifndef __MODE_SETTING__
#define __MODE_SETTING__

#include "../environment.h"
#include "../message.h"

void
mode_setting_global_init();

void
mode_setting_init();

void
mode_setting(message_buf rcv_buf);

#endif /* __MODE_SETTING__ */
