#include "./main_process.h"
#include "../../lib/fpga_dot_font.h"
#include "../prog/mode_clock.h"

static const unsigned int minus_one = -1;

int main_process(struct environment *env) {
  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf rcv_buf, snd_buf;
  const size_t buf_length = sizeof(message_buf);

  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    {
      perror("msgget");
      exit(1);
    }

  mode_clock_global_init(env, msqid);


  /* declare and set variables for Mode5 */
  struct __mode5_flag *mode5_flag = &env->mode5_flag;
  /***************************************/

  unsigned int mode;

  /* TODO: split modes */
  /********************/

#define SET_OUT_BUF(device)\
  snd_buf.mtype = MTYPE_OUTPUT; snd_buf.mtext[0] = device;

  while (!quit) {
    // event handling
    if (msgrcv(msqid, &rcv_buf, buf_length, MTYPE_READKEY, IPC_NOWAIT) != minus_one)
      {
        unsigned int code = rcv_buf.mtext[0];

        switch (code) 
          {
          case BACK:
              {
                kill(env->pid_input, SIGINT);
                kill(env->pid_output, SIGINT);
                kill(getpid(), SIGINT);
              } break;
          case VOL_P: 
              {
                ++ mode; mode %= NUM_MODE;
              } break;
          case VOL_M:
              {
                mode += NUM_MODE-1; mode %= NUM_MODE;
              } break;
          }
        printf("Current Mode: %d\n", mode);
      }

      /*
    default:
        {
          perror("mode num is out of range");
          // TODO: STOP
          exit(1);
        }
    }
    */
  }

  int status;
  pid_t pid;

  if ((pid = waitpid(-1, &status, 0)) == -1)
    perror("wait() error");
  if ((pid = waitpid(-1, &status, 0)) == -1)
    perror("wait() error");

  destruct_environment(env);

  return 0;
}
