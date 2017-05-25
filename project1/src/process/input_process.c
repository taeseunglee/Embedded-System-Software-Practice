#include "./input_process.h"

void
input_process(struct environment *env)
{
  struct input_event ev[BUFF_SIZE];
  unsigned int press_find, i;

  /* get file descriptor from env */
  int ev_fd = env->ev_fd;
  int push_switch_fd = env->push_switch_fd;

  /* setting message queue */
  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf switch_mbuf, key_mbuf;

  size_t size_switch = sizeof(char) * MAX_BUTTON;
  size_t size_event = sizeof(struct input_event);
  size_t buf_length = sizeof(message_buf);
  size_t size_mtext = sizeof(switch_mbuf.mtext);

  char before_switch_status[MAX_BUTTON];

  /* Initialize variable */
  switch_mbuf.mtype = MTYPE_SWITCH;
  key_mbuf.mtype = MTYPE_READKEY;

  memset(switch_mbuf.mtext, 0, size_mtext);
  memset(key_mbuf.mtext, 0, size_mtext);

  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    {
      perror("msgget");
      exit(1);
    }


  memset(before_switch_status, 0x00, MAX_BUTTON);
  while (!quit)
    {
      // read an event
      if (read(ev_fd, ev, size_event * BUFF_SIZE) >= size_event) 
        {
          if (ev[0].value == KEY_PRESS)
            {
              ev[0].value = KEY_RELEASE; // prevent to read duplicate data
              key_mbuf.mtext[0] = ev[0].code;
              MSGSND_OR_DIE(msqid, &key_mbuf, buf_length, IPC_NOWAIT);
            }
        }
      usleep(100000);

      /* read push-switch */
      read(push_switch_fd, &(switch_mbuf.mtext), size_switch);
      press_find = FALSE, i = 8;
      do
        {
          if (switch_mbuf.mtext[i])
            {
              press_find = TRUE;
              break;
            }
        } while(i--);

      if (press_find)
        {
          MSGSND_OR_DIE(msqid, &switch_mbuf, buf_length, IPC_NOWAIT);
          memset(switch_mbuf.mtext, 0x00, size_mtext);
        }
    }

  printf("Input Process exit\n");

  exit(0);
}
