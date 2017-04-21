#include "./main_process.h"

#include "../prog/mode_clock.h"
#include "../prog/mode_counter.h"
#include "../prog/mode_text_editor.h"
#include "../prog/mode_draw_board.h"
#include "../prog/mode_setting.h"

static const unsigned int minus_one = -1;

static void
all_mode_global_init(struct environment *env, int msqid)
{
  mode_clock_global_init      (env, msqid);
  mode_counter_global_init    (env, msqid);
  mode_text_editor_global_init(env, msqid);
  mode_draw_board_global_init (env, msqid);
  mode_setting_global_init    (env, msqid);
}

typedef void (*mode_init_func) (void);

static mode_init_func*
all_mode_init_function_pointer_init(void)
{
  mode_init_func *mode_init = calloc (NUM_MODE,
                                      sizeof(mode_init_func));
  
  mode_init[0] = mode_clock_init;
  mode_init[1] = mode_counter_init;
  mode_init[2] = mode_text_editor_init;
  mode_init[3] = mode_draw_board_init;
  mode_init[4] = mode_setting_init;

  return mode_init;
}


int
main_process(struct environment *env) {
  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf rcv_buf;
  const size_t buf_length = sizeof(message_buf);
  unsigned int code;
  mode_init_func *mode_init = NULL;

  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    {
      perror("msgget");
      exit(1);
    }

  all_mode_global_init(env, msqid);
  mode_init = all_mode_init_function_pointer_init();

  mode_init[0]();
  while (!quit) {
    // event handling
    if (msgrcv(msqid, &rcv_buf, buf_length, MTYPE_READKEY, IPC_NOWAIT) != minus_one)
      {
        code = rcv_buf.mtext[0];

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
                ++ env->mode;
                env->mode %= NUM_MODE;
                mode_init[env->mode]();
              } break;
          case VOL_M:
              {
                env->mode += NUM_MODE-1;
                env->mode %= NUM_MODE;
                mode_init[env->mode]();
              } break;
          }
        printf("Current Mode: %d\n", env->mode);
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
