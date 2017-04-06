#include "./input_process.h"

void input_process(struct environment *env)
{
  struct input_event ev[BUFF_SIZE];
  size_t ret_bytes;
  /* declare file descriptions */
  int value, size_event = sizeof(struct input_event);
  unsigned press_find = FALSE, i;

  /* get file descriptor from env */
  int ev_fd = env->ev_fd,
      push_switch_fd = env->push_switch_fd;

  /* setting message queue */
  int msqid, msgflg = IPC_CREAT | 0666;
  key_t key;
  message_buf mbuf;
  unsigned char before_switch[MAX_BUTTON] = {0,};


  memset(mbuf.mtext, 0, sizeof(mbuf.mtext));
  size_t buf_length = sizeof(message_buf); 

  key = 1234;
  if ((msqid = msgget(key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }

  /* variables for push swtich */
  size_t sw_buff_size = sizeof(char) * MAX_BUTTON;


  while (!quit) {
    if ((ret_bytes = read(ev_fd, ev, size_event * BUFF_SIZE)) >= size_event) {
      if (ev[0].value == KEY_PRESS) {
        ev[0].value = KEY_RELEASE;
        mbuf.mtype = MTYPE_READKEY;
        mbuf.mtext[0] = ev[0].code;

        // send message
        if (msgsnd(msqid, &mbuf, buf_length, IPC_NOWAIT) < 0) {
          printf("%d, %ld, %d, %zu\n", msqid, mbuf.mtype, mbuf.mtext[0], buf_length);
          perror("msgsnd event");
          kill(getppid(), SIGINT);
          exit(1);
        }
      }
    }

    usleep(200000);

    read(push_switch_fd, &(mbuf.mtext), sw_buff_size);

    press_find = FALSE, i = 8;
    do {
      if (mbuf.mtext[i] && before_switch[i]) {
        press_find = TRUE;
        break;
      }
    } while(i--);
    memcpy(before_switch, mbuf.mtext, sw_buff_size);

    if (press_find) {
      mbuf.mtype = MTYPE_SWITCH;
      if (msgsnd(msqid, &mbuf, buf_length, IPC_NOWAIT) < 0) {
        printf("%d, %ld, %zu\n", msqid, mbuf.mtype, buf_length);
        perror("msgsnd buffer");
        exit(1);
      }
    }
  }

  printf("Input Process exit\n");
}

