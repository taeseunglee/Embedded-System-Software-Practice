#include "./output_process.h"

#define WRITE_ERROR_CHECK(fd, rcv_data, wbyte) \
  ({ \
  if (write(fd, rcv_data, wbyte) < 0) \
      perror("write"); \
   })

void output_process(struct environment *env)
{
  // get file descriptor from env
  int dot_fd = env->dot_fd;
  int led_fd = env->led_fd;
  int fnd_fd = env->fnd_fd;
  int lcd_fd = env->lcd_fd;

  int is_running_program = TRUE;
  int id_type;
  unsigned char rcv_data[20];

  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf rcv_buf;

  /* setting message queue */
  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    perror("msgget");

  while (is_running_program)
    {
      if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_OUTPUT, 0))
        {
          id_type = rcv_buf.mtext[0];

          switch (id_type)
            {
            case ID_DOT:
                {
                  memcpy(rcv_data, rcv_buf.mtext+1, LEN_DOT);
                  WRITE_ERROR_CHECK(dot_fd, rcv_data, LEN_DOT);
                } break;
            case ID_LED:
                {
                  rcv_data[0] = rcv_buf.mtext[1];
                  WRITE_ERROR_CHECK(led_fd, &rcv_data, LEN_LED);
                } break;
            case ID_FND: 
                {
                  memcpy(rcv_data, rcv_buf.mtext+1, LEN_FND);
                  WRITE_ERROR_CHECK(fnd_fd, &rcv_data, LEN_FND);
                } break;
            case ID_LCD:
                {
                  memcpy(rcv_data, rcv_buf.mtext+1, LEN_LCD);
                  WRITE_ERROR_CHECK(lcd_fd, rcv_data, LEN_LCD);
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
            case END_PROGRAM:
                {
                  is_running_program = FALSE;
                } break;
            default:
                {
                  // TODO: error
                  is_running_program = FALSE;
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

  write(env->led_fd, blank_data, LEN_LED);
  write(env->fnd_fd, blank_data, LEN_FND);
  write(env->lcd_fd, blank_data, LEN_LCD);
  write(env->dot_fd, blank_data, LEN_DOT);
}
