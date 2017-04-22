#include "./mode_counter.h"
/* declare and set variables for Mode2 */
static unsigned int idx_base, count;

/* Variablefor message queue */
static int msqid;
static message_buf snd_buf;
static const size_t buf_length = sizeof(message_buf);
static struct environment *env;

/* base 10, do not use
   digit array. */
static const unsigned int led_num[] = { 64, 32, 16, 128 };
static const unsigned int digit[4][4] = 
{
  [1] = {0xFFF, 0x1FF, 0x3F, 0x07},
  [2] = {0xFF, 0x3F, 0x0F, 0x03},
  [3] = {0x0F, 0x07, 0x03, 0x01}
};
static const unsigned int num_up[4][2] = 
{
    {100, 10}, {64, 8},
    {16, 4}, {4, 2}
};
/* REMOVE  base[] = {10, 8, 4, 2}, */

static const unsigned int base_shift[3][4] = 
{
    { 0, 3, 2, 1 },
    { 0, 6, 4, 2 },
    { 0, 9, 6, 3 }
};
/***************************************/


void
mode_counter_global_init (struct environment * __env, int __msqid)
{
  msqid = __msqid;
  env   = __env;
}

void
mode_counter_init(void)
{
  idx_base = 0, count = 0;

  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  snd_buf.mtext[1] = led_num[0];
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
  memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
}

void
mode_counter(message_buf rcv_buf)
{
  if (rcv_buf.mtext[0])
    { /* change base */
      ++idx_base;
      idx_base &= 0x03;

      snd_buf.mtext[1] = led_num[idx_base];
      set_out_buf(snd_buf, ID_LED);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
  else if (rcv_buf.mtext[1]) /* Increase the second number */
    count += num_up[idx_base][0];
  else if (rcv_buf.mtext[2]) /* Increase the third number */
    count += num_up[idx_base][1];
  else if (rcv_buf.mtext[3]) /* Increase the fourth number */
    ++count;

  count %= 10000;

  // Update the Count
  if (idx_base) 
    { /* not base-10 */
      snd_buf.mtext[4] = (count & digit[idx_base][3]);
      snd_buf.mtext[3] = (count & digit[idx_base][2])>>base_shift[0][idx_base];
      snd_buf.mtext[2] = (count & digit[idx_base][1])>>base_shift[1][idx_base];
      snd_buf.mtext[1] = (count & digit[idx_base][0])>>base_shift[2][idx_base];
    }
  else 
    { /* base-10 */
      snd_buf.mtext[4] = count%10;
      snd_buf.mtext[3] = (count%100)/10;
      snd_buf.mtext[2] = (count%1000)/100;
      if (env->mode5_flag.mode_4th_of_base10) snd_buf.mtext[1] = count/1000;
      else snd_buf.mtext[1] = 0;
    }

  /* print out counter */
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}
