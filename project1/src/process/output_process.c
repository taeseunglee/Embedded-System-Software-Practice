#include "./output_process.h"

#define WRITE_ERROR_CHECK(fd, rcv_data, wbyte) \
  if (write(fd, rcv_data, wbyte) < 0) \
    {\

    }

void output_process(struct environment *env)
{
  // get file descriptor from env
  int dot_fd = env->dot_fd;
  int led_fd = env->led_fd;
  int fnd_fd = env->fnd_fd;
  int lcd_fd = env->lcd_fd;

  int id_type;
  unsigned char rcv_data[20];

  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf rcv_buf;

  /* setting message queue */
  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    perror("msgget");

  while (!quit)
    {
      if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_OUTPUT, 0))
        {
          id_type = rcv_buf.mtext[0];

          // TODO: Check "Write Error"
          switch (id_type)
            {
            case ID_DOT:
                {
                  memcpy(rcv_data, rcv_buf.mtext+1, MAX_DOT);
                  write(dot_fd, rcv_data, MAX_DOT); // TODO: WRITE ERROR CHECK
                } break;
            case ID_LED:
                {
                  rcv_data[0] = rcv_buf.mtext[1];
                  write(led_fd, &rcv_data, 1);
                } break;
            case ID_FND: 
                {
                  memcpy(rcv_data, rcv_buf.mtext+1, 4);
                  write(fnd_fd, &rcv_data, 4);
                } break;
            case ID_LCD:
                {
                  memcpy(rcv_data, rcv_buf.mtext+1, MAX_TEXT);
                  write(lcd_fd, rcv_data, MAX_TEXT);
                } break;
            case ID_MOTOR:
                {
                  // not use. blank
                } break;
            case ID_DIP_SW:
                {
                  // not use. blank
                } break;
            case ID_BUZZER:
                {
                  // not use. blank
                } break;
            case DEVICE_CLEAR:
                {
                  device_clear(env);
                } break;
            }
        }
    }
  device_clear(env);

  printf("Output Process exit\n");
  exit(0);
}

void device_clear(struct environment *env)
{
  static unsigned blank_data[50] = {0,};

  write(env->led_fd, blank_data, 1);
  write(env->fnd_fd, blank_data, 4);
  write(env->lcd_fd, blank_data, MAX_TEXT);
  write(env->dot_fd, blank_data, MAX_DOT);
}
