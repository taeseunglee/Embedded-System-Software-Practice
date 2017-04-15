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



  /* declare and set variables for overall Mode */
  int need_init = TRUE; // TODO: INIT check when after Change
//  unsigned int mode = 0; // mode 1~NUM_MODE




  /* declare and set variables for Mode5 */
  struct __mode5_flag *mode5_flag = &env->mode5_flag;
  /***************************************/


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
                need_init = TRUE;
              } break;
          case VOL_M:
              {
                mode += NUM_MODE-1; mode %= NUM_MODE;
                need_init = TRUE;
              } break;
          }
        printf("Current Mode: %d\n", mode);
      }


    switch(mode) {


    case 4:
        {
          if (need_init) {
            SET_OUT_BUF(DEVICE_CLEAR);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            need_init = FALSE;

            snd_buf.mtext[1] = 8 | (128*mode5_flag->mode_time_goes | 64*mode5_flag->mode_4th_of_base10);

            SET_OUT_BUF(ID_LED);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            count = 0;
          }

          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one) {
            if (rcv_buf.mtext[0]) {
              mode5_flag->mode_time_goes ^= 1;
              env->begin = clock();
              ++ count;
            }
            if (rcv_buf.mtext[1]) {
              mode5_flag->mode_4th_of_base10 ^= 1;
              ++ count;
            }

// TODO: Using #define, #undefine, and ##(concatenate) in #define, refactoring below code.

            snd_buf.mtext[1] = 8 | (128*mode5_flag->mode_time_goes | 64*mode5_flag->mode_4th_of_base10);
            SET_OUT_BUF(ID_LED);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
            snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
            SET_OUT_BUF(ID_FND);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
          }
        } break;


    default:
        {
          perror("mode num is out of range");
          // TODO: STOP
          exit(1);
        }
    }
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
