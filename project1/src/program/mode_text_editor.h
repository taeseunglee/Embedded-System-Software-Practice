#ifndef __MODE_TEXT_EDITOR__
#define __MODE_TEXT_EDITOR__

#include "../process/process.h"
#include "../lib/environment.h"
#include "../lib/message.h"

void mode_text_editor_global_init(struct environment *__env, int __msqid);
void mode_text_editor_init(void);
void mode_text_editor(message_buf rcv_buf);

#endif /* __MODE_TEXT_EDITOR__ */
