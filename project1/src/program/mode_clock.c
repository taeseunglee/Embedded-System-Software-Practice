#include "./mode_clock.h"
#include "../process/process.h"

/* static global variable */
static unsigned int cur_hour, cur_min;

/* Variablefor message queue */
static int msqid;
static message_buf snd_buf;
static const size_t buf_length = sizeof(message_buf);
static struct environment *env;

static pthread_t flicker_thread;
static int is_led_flick_on = 0;
static unsigned int cur_led;
static unsigned int time_second;

/* In mode1, this is called to main process as a start routine of pthread.
 * LEDs come out alternately every second.
 */
void* led_flicker_handler(void *arguments)
{
  int led_fd = env->led_fd;

  int i = 0;
  unsigned char led_data;

  printf("led_flicker_handler on\n");

  while (is_led_flick_on)
    {
      cur_led = 128 | 32;
      led_data = 128 | 32 | time_second; write(led_fd, &led_data, 1);
      i = 4;
      do
        { 
          usleep(245000);
          if (!is_led_flick_on) break;
        } while(--i);

      cur_led = 16;
      led_data = 128 | 16 | time_second; write(led_fd, &led_data, 1);
      i = 4;
      do
        { 
          usleep(245000);
          if (!is_led_flick_on) break;
        } while(--i);
    }
  led_data = 128 | time_second;

  printf("led_flicker_handler terminated\n");
  pthread_exit(NULL);
}

static void
set_init_time(void)
{
  /* time setting */
  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime (&rawtime);

  if (!timeinfo)
    {
      // TODO: kill all processes
      perror("Localtime Error");
    }
  else
    {
      cur_hour = timeinfo->tm_hour;
      cur_min = timeinfo->tm_min;
    }
}


void
mode_clock_global_init (struct environment * __env, int __msqid)
{
  msqid = __msqid;
  env   = __env;

  set_init_time();
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

  // print out the current time at FND
  cur_hour %= 24;
  snd_buf.mtext[1] = cur_hour/10; snd_buf.mtext[2] = cur_hour%10;
  snd_buf.mtext[3] = cur_min/10;  snd_buf.mtext[4] = cur_min%10;
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}

void
mode_clock_exit(void)
{
  if (is_led_flick_on)
    {
      is_led_flick_on = FALSE;
      pthread_join(flicker_thread, NULL);
    }
  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}

void 
mode_clock(message_buf rcv_buf)
{
  /* declare and set variables for Mode1 */
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
      is_led_flick_on ^= 1;
      if (is_led_flick_on
          && (pthread_create(&flicker_thread, NULL, &led_flicker_handler, NULL) != 0)
          )
        {
          perror("pthread_create");
        }
      if (!is_led_flick_on)
        {
          pthread_join(flicker_thread, NULL);

          cur_led = 128;
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
  if (is_led_flick_on)
    {
      // message get
      if (rcv_buf.mtext[1]) set_init_time();
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
