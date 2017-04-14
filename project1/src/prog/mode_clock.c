#include "mode_clock.h"
#include "../process/process.h"

/* static global variable */
static unsigned int cur_hour, cur_min;

/* For message queue */
static int msqid;
static message_buf rcv_buf, snd_buf;
static const size_t buf_length = sizeof(message_buf);
static unsigned int cur_led;
static int led_flick;
static struct environment *env;
static unsigned int time_second;
static struct argu_led_flick argu_flick = {
    .led_flick = &led_flick,
    .cur_led = &cur_led,
    .time_second = &time_second
};


/* Functions */
void
mode_clock_global_init (struct environment * __env, int __msqid)
{
  msqid = __msqid;
  env   = __env;
  argu_flick.env = env;

  /* time setting */
  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime (&rawtime);

  if (!timeinfo)
    {
      printf("Localtime Error\n");
      // TODO: kill all processes
    }
  else
    {
      cur_hour = timeinfo->tm_hour;
      cur_min = timeinfo->tm_min;
    }
  mode_clock_init();
}

void
mode_clock_init(void)
{
  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  // set led D1
  snd_buf.mtext[1] = cur_led = 128;
  set_out_buf(snd_buf, ID_LED);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  cur_hour %= 24;
  snd_buf.mtext[1] = cur_hour/10; snd_buf.mtext[2] = cur_hour%10;
  snd_buf.mtext[3] = cur_min/10;  snd_buf.mtext[4] = cur_min%10;
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}

void 
mode_clock(message_buf rcv_buf)
{
  /* declare and set variables for Mode1 */
  pthread_t flicker_thread;
  int time_spent;
  clock_t end;


  if (env->mode5_flag.mode_time_goes)
    {
      end = clock();
      time_spent = (int) (end - env->begin) / CLOCKS_PER_SEC;

      // 1second goes
      if (time_spent)
        {
          env->begin = end;
          ++ time_second;

          snd_buf.mtext[1] = cur_led | time_second;
          set_out_buf(snd_buf, ID_LED);
          MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
          if (time_second/60)
            {
              cur_min += time_second/60;
              time_second %= 60;

              snd_buf.mtext[1] = cur_hour/10;
              snd_buf.mtext[2] = cur_hour%10;
              snd_buf.mtext[3] = cur_min/10;
              snd_buf.mtext[4] = cur_min%10;

              set_out_buf(snd_buf, ID_FND);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }
        }
    }
  // Modify time in board
  if (rcv_buf.mtext[0]) 
    {
      led_flick ^= 1;
      if (led_flick
          && (pthread_create(&flicker_thread, NULL, &led_flicker,
                             (void*) &argu_flick) != 0)
          )
        {
          perror("pthread_create");
          // TODO: kill all processes or just notice error occurunce.
        }
      if (!led_flick)
        {
          cur_led = 128;
          usleep(450000);
          
          snd_buf.mtext[1] = cur_led | time_second;
          set_out_buf(snd_buf, ID_LED);
          MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
        }
    }
  // Reset time in board
  else if (rcv_buf.mtext[1])
    {
      memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));

      set_out_buf(snd_buf, ID_FND);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
  if (led_flick)
    {
      // message get
      if (rcv_buf.mtext[2]) ++ cur_hour;
      if (rcv_buf.mtext[3]) ++ cur_min;

      cur_hour %= 24;

      // send fnd data
      snd_buf.mtext[1] = cur_hour/10; snd_buf.mtext[2] = cur_hour%10;
      snd_buf.mtext[3] = cur_min/10;  snd_buf.mtext[4] = cur_min%10;
      set_out_buf(snd_buf, ID_FND);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
}
