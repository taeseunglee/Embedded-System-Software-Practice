#include "./mode_setting.h"
#include "../process/process.h"

/* Variablefor message queue */
static int msqid;
static message_buf snd_buf;
static const size_t buf_length = sizeof(message_buf);
static struct environment *env;

static unsigned int count;

void
mode_setting_global_init(struct environment *__env, int __msqid)
{
  env = __env;
  msqid = __msqid;
}

void
mode_setting_init(void)
{
  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  snd_buf.mtext[1] = 8 | (128*env->mode5_flag.mode_time_goes | 64*env->mode5_flag.mode_4th_of_base10);

  set_out_buf(snd_buf, ID_LED);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  count = 0;
}

void
mode_setting_exit()
{
   set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}

void
mode_setting(message_buf rcv_buf)
{
  if (rcv_buf.mtext[0])
    {
      env->mode5_flag.mode_time_goes ^= 1;
      env->begin = clock();
      ++ count;
    }
  if (rcv_buf.mtext[1])
    {
      env->mode5_flag.mode_4th_of_base10 ^= 1;
      ++ count;
    }

  snd_buf.mtext[1] = 8
    | 128*env->mode5_flag.mode_time_goes
    | 64 *env->mode5_flag.mode_4th_of_base10;
  set_out_buf(snd_buf, ID_LED);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
  snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}
