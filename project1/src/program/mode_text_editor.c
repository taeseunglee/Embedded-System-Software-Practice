#include "../lib/define.h"
#include "./mode_text_editor.h"
#include "../lib/fpga_dot_font.h"


/* Variablefor message queue */
static int msqid;
static message_buf snd_buf;
static const size_t buf_length = sizeof(message_buf);
static struct environment *env;

#define SHIFT_TEXT(text) \
  do \
  { \
    int t=0; \
    while (t<LIMIT_TEXT) { text[t] = text[t+1]; t++; } \
  } while(0);



static unsigned char text[LEN_LCD];
static int count, idx_text;
static unsigned int text_mode;
// character
const unsigned char map_char[9][3] =
{
  [0] = {'.','Q','Z'}, [1] = {'A','B','C'},
  [2] = {'D','E','F'}, [3] = {'G','H','I'},
  [4] = {'J','K','L'}, [5] = {'M','N','O'},
  [6] = {'P','R','S'}, [7] = {'T','U','V'},
  [8] = {'W','X','Y'}
};
/***************************************/

void
mode_text_editor_global_init(struct environment *__env, int __msqid)
{
  env = __env;
  msqid = __msqid;
}


void
mode_text_editor_init()
{
  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  text_mode = TRUE;
  count = 0, idx_text = -1;

  snd_buf.mtext[1] = 32; // Set D3 led
  set_out_buf(snd_buf, ID_LED);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  memset(text, 0, LEN_LCD);
  memcpy(snd_buf.mtext+1, text, LEN_LCD);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  memcpy(snd_buf.mtext+1, fpga_alpha, LEN_DOT);
  set_out_buf(snd_buf, ID_DOT);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}

void
mode_text_editor_exit()
{
   set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}

/* ------------------------------------------------------------------------ */
void
mode_text_editor(message_buf rcv_buf)
{
  static int prior_pressed, times;
  if (rcv_buf.mtext[1] && rcv_buf.mtext[2])
    {
      memset(text, 0, sizeof(text));
      idx_text = -1;
      count += 2;
    }
  else if (rcv_buf.mtext[4] && rcv_buf.mtext[5])
    {
      text_mode ^= 1;

      if (text_mode)
        {
          memcpy(snd_buf.mtext+1, fpga_alpha, LEN_DOT);
          set_out_buf(snd_buf, ID_DOT);
          MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
        }
      else
        {
          memcpy(snd_buf.mtext+1, fpga_number[1], LEN_DOT);
          set_out_buf(snd_buf, ID_DOT);
          MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
        }

      count += 2;
    }
  else if (rcv_buf.mtext[7] && rcv_buf.mtext[8])
    {
      if (idx_text != LIMIT_TEXT)
        ++ idx_text;
      else
        SHIFT_TEXT(text);
      text[idx_text] = ' ';

      count += 2;
    }
  else
    { // write text into lcd
      unsigned int i = 8;
      do
        {
          if (!rcv_buf.mtext[i])
            continue;

          if (prior_pressed != i)
            {
              prior_pressed = i;
              times = 0;
            }
          else
            {
              ++ times;
              times %= 3;
            }

          if (text_mode)
            { // Text Mode
              if (times) // exchange alphabet because of re-pushing
                text[idx_text] = map_char[i][times];
              else 
                { // add a new character.
                  if (idx_text != LIMIT_TEXT)
                    ++ idx_text;
                  else
                    SHIFT_TEXT(text);

                  text[idx_text] = map_char[i][0];
                }
            }
          else
            { // Number Mode
              if (idx_text != LIMIT_TEXT) ++ idx_text;
              else SHIFT_TEXT(text);

              text[idx_text] = i+0x31;
            }

          ++ count;
          break;
        }
      while(i--);
    }

  count %= 10000;

  snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
  snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  memcpy(snd_buf.mtext+1, text, LEN_LCD);
  set_out_buf(snd_buf, ID_LCD);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}
