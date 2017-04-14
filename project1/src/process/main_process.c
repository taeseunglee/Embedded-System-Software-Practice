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

  mode_clock_global_init(msqid);



  /* declare and set variables for overall Mode */
  int need_init = TRUE; // TODO: INIT check when after Change
  unsigned int mode = 0; // mode 1~NUM_MODE

  /* declare and set variables for Mode2 */
  unsigned int idx_base, count;

  /* base 10, do not use
     digit array. */
  const unsigned int
    led_num[] = { 64, 32, 16, 128 },
    digit[4][4] = 
      {
        [1] = {0xFFF, 0x1FF, 0x3F, 0x07},
        [2] = {0xFF, 0x3F, 0x0F, 0x03},
        [3] = {0x0F, 0x07, 0x03, 0x01}
      },
    num_up[4][2] = 
      {
        {100, 10}, {64, 8},
        {16, 4}, {4, 2}
      },
    base[] = {10, 8, 4, 2},
    base_shift[3][4] = 
      {
        { 0, 3, 2, 1 },
        { 0, 6, 4, 2 },
        { 0, 9, 6, 3 }
      };
  /***************************************/

  /* declare and set variables for Mode3 */
  unsigned char text[MAX_TEXT], idx_text = -1;
  unsigned int text_mode = 1, i;
  // character
  const unsigned char map_char[9][3] =
    {
        [0] = {'.','Q','Z'}, [1] = {'A','B','C'},
        [2] = {'D','E','F'}, [3] = {'G','H','I'},
        [4] = {'J','K','L'}, [5] = {'M','N','O'},
	[6] = {'P','R','S'}, [7] = {'T','U','V'},
        [8] = {'W','X','Y'}
    };
  int prior_pressed, times;
  /***************************************/

  /* declare and set variables for Mode4 */
  struct cursor cursor;
  pthread_t cursor_thread;
  unsigned int cursor_hide = 0;
  unsigned char mask[10] = {0};
  const size_t size_mask = 10*sizeof(unsigned char);
  struct argu_mode_cursor argu_cursor =
    {
      .cursor = &cursor,
      .mask = snd_buf.mtext+1,
      .mode = &mode,
      .cursor_hide = &cursor_hide,
      .env = env
    };
  /***************************************/

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
//                mode_change_time = 0; // TODO: same feature with led flick
              } break;
          case VOL_M:
              {
                mode += NUM_MODE-1; mode %= NUM_MODE;
                need_init = TRUE;
 //               mode_change_time = 0; // TODO: same feature with led flick
              } break;
          }
        printf("Current Mode: %d\n", mode);
      }


    switch(mode) {
     case 1:
        {
          if (need_init)
            {
              SET_OUT_BUF(DEVICE_CLEAR);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

              need_init = FALSE;
              idx_base = 0, count = 0;

              snd_buf.mtext[1] = led_num[0];
              SET_OUT_BUF(ID_LED);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

              memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
              SET_OUT_BUF(ID_FND);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }

          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one) {
            if (rcv_buf.mtext[0]) {
              // change base
              ++idx_base;
              idx_base &= 0x03;

              snd_buf.mtext[1] = led_num[idx_base];
              SET_OUT_BUF(ID_LED);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }
            else if (rcv_buf.mtext[1]) /* Increase the second number */
              count += num_up[idx_base][0];
	        else if (rcv_buf.mtext[2]) /* Increase the third number */
              count += num_up[idx_base][1];
    	    else if (rcv_buf.mtext[3]) /* Increase the fourth number */
              ++count;

            // Update the Count
            if (idx_base) { // not base-10
              snd_buf.mtext[4] = (count & digit[idx_base][3]);
              snd_buf.mtext[3] = (count & digit[idx_base][2])>>base_shift[0][idx_base];
              snd_buf.mtext[2] = (count & digit[idx_base][1])>>base_shift[1][idx_base];
              snd_buf.mtext[1] = (count & digit[idx_base][0])>>base_shift[2][idx_base];
            }
            else { // base-10
              snd_buf.mtext[4] = count%10;
              snd_buf.mtext[3] = (count%100)/10;
              snd_buf.mtext[2] = (count%1000)/100;
              if (mode5_flag->mode_4th_of_base10) snd_buf.mtext[1] = count/1000;
              else snd_buf.mtext[1] = 0;
            }

            SET_OUT_BUF(ID_FND);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            count %= 10000;
          }
        } break; // End of Mode 2


    case 2:
        {
          if (need_init)
            {
              SET_OUT_BUF(DEVICE_CLEAR);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

              need_init = FALSE;
              text_mode = TRUE;
              count = 0, idx_text = -1;

              snd_buf.mtext[1] = 32; // Set D3 led
              SET_OUT_BUF(ID_LED);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

              memset(text, 0, sizeof(text));
              memcpy(snd_buf.mtext+1, text, MAX_TEXT);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

              memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
              SET_OUT_BUF(ID_FND);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

              memcpy(snd_buf.mtext+1, fpga_alpha, sizeof(fpga_alpha));
              SET_OUT_BUF(ID_DOT);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }

          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, 11, IPC_NOWAIT) != minus_one) {
            if (rcv_buf.mtext[1] && rcv_buf.mtext[2]) {
              memset(text, 0, sizeof(text));
              idx_text = -1;
              count += 2;
            }
            else if (rcv_buf.mtext[4] && rcv_buf.mtext[5]) {
              text_mode ^= 1;

              int str_size = sizeof(fpga_alpha);
              if (text_mode) {
                memcpy(snd_buf.mtext+1, fpga_alpha, 10*sizeof(char));
                SET_OUT_BUF(ID_DOT);
                MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
              }
              else {
                memcpy(snd_buf.mtext+1, fpga_number[1], 10*sizeof(char));
                SET_OUT_BUF(ID_DOT);
                MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
              }

              count += 2;
            }
            else if (rcv_buf.mtext[7] && rcv_buf.mtext[8]) {
              if (idx_text != LIMIT_TEXT) ++ idx_text;
              else SHIFT_TEXT(text);
              text[idx_text] = ' ';
              
              count += 2;
            }
            else { // write text into lcd
              i = 8;
              do {
                if (rcv_buf.mtext[i]) {
                  if (prior_pressed != i) {
                    prior_pressed = i;
                    times = 0;
                  }
                  else { ++ times; times %= 3; }

                  if (text_mode) { // Text Mode
                    if (times) // exchange alphabet because of re-pushing
                      text[idx_text] = map_char[i][times];
                    else { // add a new character.
                      if (idx_text != LIMIT_TEXT) ++ idx_text;
                      else SHIFT_TEXT(text);

                      text[idx_text] = map_char[i][0];
                    }
                  }
                  else { // Number Mode
                    if (idx_text != LIMIT_TEXT) ++ idx_text;
                    else SHIFT_TEXT(text);

                    text[idx_text] = i+0x31;
                  }
                  ++ count;
                  break;
                }
              } while(i--);
            }

            snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
            snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
            SET_OUT_BUF(ID_FND);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            memcpy(snd_buf.mtext+1, text, MAX_TEXT);
            SET_OUT_BUF(ID_LCD);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            count %= 10000;
          }
        } break; // End of Mode 3


    case 3:
        {
          if (need_init) {
            SET_OUT_BUF(DEVICE_CLEAR);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            need_init = FALSE;

            snd_buf.mtext[1] = 16; // Set D4 led
            SET_OUT_BUF(ID_LED);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            memset(snd_buf.mtext+1, 0, size_mask);
            SET_OUT_BUF(ID_DOT);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

            cursor.x = 1, cursor.y = 0;

            count = 0;
            if (pthread_create(&cursor_thread, NULL, &print_cursor, (void*)&argu_cursor) != 0) {
              printf("fail: pthread_create\n");
              return -1;
            }
          }
          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one) {
            /* Move the cursor */
            if(rcv_buf.mtext[1]) {
              if (cursor.y) -- cursor.y;
              ++ count;
            }
            if(rcv_buf.mtext[3]) {
              if (cursor.x > 1) -- cursor.x;
              ++ count;
            }
            if(rcv_buf.mtext[5]) {
              if (!(cursor.x > 7)) ++ cursor.x;
              ++ count;
            }
            if(rcv_buf.mtext[7]) {
              if (!(cursor.y > 9)) ++ cursor.y;
              ++ count;
            }

            /* Modify the board setting */
            if (rcv_buf.mtext[0]) {
              cursor.x = 1, cursor.y = 0;
              memset(snd_buf.mtext+1, 0x00, sizeof(mask));

              ++ count;
              SET_OUT_BUF(ID_DOT);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }
            if (rcv_buf.mtext[2]) {
              cursor_hide ^= 1;
              ++ count;
            }
            // TODO: 0x80 --> 0x40 && init cursor.x = 1 --> cursor.x = 0;
            if (rcv_buf.mtext[4]) {
              // select and toggle the point
              snd_buf.mtext[cursor.y+1] ^= (0x80 >> cursor.x);
              ++ count;
              SET_OUT_BUF(ID_DOT);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }
            if (rcv_buf.mtext[6]) {
              memset(snd_buf.mtext+1, 0x00, sizeof(mask));
              ++ count;
              SET_OUT_BUF(ID_DOT);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }
            if (rcv_buf.mtext[8]) {
              // Invert the board
              int i = 10;
              do {
                snd_buf.mtext[i] ^= 0xFF;
              } while(--i);
              ++ count;
              SET_OUT_BUF(ID_DOT);
              MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            }

// TODO: Use mask!! --> change "Use directly snd_buf.mtext" to "Use mask as a field and copy this to mtext"
            memcpy(mask, snd_buf.mtext+1, size_mask);
            snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
            snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
            SET_OUT_BUF(ID_FND);
            MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
            memcpy(snd_buf.mtext+1, mask, size_mask);

            count %= 10000;
          }
        } break; // End of Mode 4

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
