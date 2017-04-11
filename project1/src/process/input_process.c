#include "./input_process.h"

void
input_process(struct environment *env)
{
  struct input_event ev[BUFF_SIZE];
  size_t ret_bytes;
  unsigned press_find = FALSE, i;
  size_t sw_buff_size = sizeof(char) * MAX_BUTTON,
         size_event = sizeof(struct input_event);


  /* get file descriptor from env */
  int ev_fd = env->ev_fd,
      push_switch_fd = env->push_switch_fd;

  /* setting message queue */
  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf mbuf;
  unsigned char before_switch[MAX_BUTTON] = {0,};
  memset(mbuf.mtext, 0, sizeof(mbuf.mtext));
  size_t buf_length = sizeof(message_buf); 

  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    {
      perror("msgget");
      exit(1);
    }


  while (!quit)
    {
      // read an event
      if ((ret_bytes = read(ev_fd, ev, size_event * BUFF_SIZE)) >= size_event) 
        {
          if (ev[0].value == KEY_PRESS)
            {
              ev[0].value = KEY_RELEASE; // prevent multiple 
              mbuf.mtext[0] = ev[0].code; mbuf.mtype = MTYPE_READKEY;
              MSGSND_OR_DIE(msqid, &mbuf, buf_length, IPC_NOWAIT);
            }
        }
      usleep(100000);

      read(push_switch_fd, &(mbuf.mtext), sw_buff_size);

      press_find = FALSE, i = 8;
      do
        {
          if (mbuf.mtext[i] && before_switch[i]) 
            {
              press_find = TRUE;
              break;
            }
        } while(i--);
      memcpy(before_switch, mbuf.mtext, sw_buff_size);

      if (press_find) 
        {
          mbuf.mtype = MTYPE_SWITCH;
          MSGSND_OR_DIE(msqid, &mbuf, buf_length, IPC_NOWAIT);
        }
    }

  printf("Input Process exit\n");

  exit(0);
}
