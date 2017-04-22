#include "./main_process.h"

#include "../program/mode_clock.h"
#include "../program/mode_counter.h"
#include "../program/mode_text_editor.h"
#include "../program/mode_draw_board.h"
#include "../program/mode_setting.h"


typedef void (*mode_init_func) (void);
typedef void (*mode_body_func) (message_buf);


static void
set_mode_global_init(struct environment *env, int msqid);
static mode_init_func*
get_mode_init(void);
static mode_body_func*
get_mode_body(void);
int
main_process(struct environment *env);

int
main_process(struct environment *env)
{
  int msqid, msgflg = IPC_CREAT | 0666;
  message_buf rcv_buf;
  const size_t buf_length = sizeof(message_buf);
  unsigned int code;
  mode_init_func *mode_init = NULL;
  mode_body_func *mode_body = NULL;
  const unsigned int minus_one = -1;

  if ((msqid = msgget(env->msg_key, msgflg)) < 0)
    {
      perror("msgget");
      exit(1);
    }

  set_mode_global_init(env, msqid);
  mode_init = get_mode_init();
  mode_body = get_mode_body();

  if (!mode_body || !mode_init)
    kill_all_processes(env);

  mode_init[env->mode]();
  while (!quit)
    {
      // event handling
      if (msgrcv(msqid, &rcv_buf, buf_length, MTYPE_READKEY, IPC_NOWAIT) != minus_one)
        {
          code = rcv_buf.mtext[0];

          switch (code) 
            {
            case BACK:
                {
                  kill_all_processes(env);
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
      if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one)
        mode_body[env->mode](rcv_buf);
    }


  /* Terminate the program */
  int status;
  pid_t pid;

  if ((pid = waitpid(-1, &status, 0)) == -1)
    perror("wait() error");
  if ((pid = waitpid(-1, &status, 0)) == -1)
    perror("wait() error");

  free(mode_init);
  free(mode_body);

  destruct_environment(env);

  return 0;
}

static void
set_mode_global_init(struct environment *env, int msqid)
{
  mode_clock_global_init      (env, msqid);
  mode_counter_global_init    (env, msqid);
  mode_text_editor_global_init(env, msqid);
  mode_draw_board_global_init (env, msqid);
  mode_setting_global_init    (env, msqid);
}


static mode_init_func*
get_mode_init(void)
{
  mode_init_func *mode_init = calloc (NUM_MODE,
                                      sizeof(mode_init_func));
 
  if (!mode_init)
    return NULL;

  mode_init[0] = &mode_clock_init;
  mode_init[1] = &mode_counter_init;
  mode_init[2] = &mode_text_editor_init;
  mode_init[3] = &mode_draw_board_init;
  mode_init[4] = &mode_setting_init;

  return mode_init;
}


static mode_body_func*
get_mode_body(void)
{
  mode_body_func *mode_body = calloc (NUM_MODE,
                                      sizeof(mode_body_func));
  
  if (!mode_body)
    return NULL;

  mode_body[0] = &mode_clock;
  mode_body[1] = &mode_counter;
  mode_body[2] = &mode_text_editor;
  mode_body[3] = &mode_draw_board;
  mode_body[4] = &mode_setting;

  return mode_body;
}
