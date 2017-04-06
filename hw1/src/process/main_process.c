#include "src/process/process.h"

int main_process(struct environment *env) {
  // main process
  int msqid, msgflg = IPC_CREAT | 0666;
  key_t key;
  message_buf rcv_buf, snd_buf;
  const size_t buf_length = sizeof(message_buf);


  key = 1234;
  if ((msqid = msgget(key, msgflg)) < 0) {
    perror("msgget");
    exit(1);
  }



  /* declare and set variables for All Mode */
  int need_init = TRUE;
  int mode = 0, mode_minus = NUM_MODE-1; // mode 1~NUM_MODE

  /* declare and set variables for Mode1 */
  pthread_t flicker_thread;
  struct argu_led_flick argu_flick;
  argu_flick.env = &env;
  time_t rawtime;
  time(&rawtime);
  struct tm *timeinfo = localtime (&rawtime);
  unsigned int cur_hour, cur_min;

  int mode_change_time = 0;

  if (!timeinfo) {
    printf("Localtime Error\n");
    // TODO: kill all processes
  }
  else {
    cur_hour = timeinfo->tm_hour;
    cur_min = timeinfo->tm_min;
  }
  /**********************************************/

  /* declare and set variables for Mode2 */
  unsigned int idx_base, count;

  /* base 10, do not use
     digit array. */
  const unsigned int led_num[] = {64, 32, 16, 128},
        digit[4][4] = {
            {},
            {0xFFF, 0x1FF, 0x3F, 0x07},
            {0xFF, 0x3F, 0x0F, 0x03},
            {0x0F, 0x07, 0x03, 0x01}
        },
               num_up[4][2] = {
                   {100, 10}, {64, 8},
                   {16, 4}, {4, 2}
               },
               base[] = {10, 8, 4, 2},
               base_shift[3][4] = {
                   { 0, 3, 2, 1 },
                   { 0, 6, 4, 2 },
                   { 0, 9, 6, 3 }
               };

  /***************************************/

  /* declare and set variables for Mode3 */
  unsigned char text[MAX_TEXT], idx_text = -1;
  unsigned int text_mode = 1, i;
  // character
  const unsigned char	
	map_char[9][3] = {
		  {'.','Q','Z'}, {'A','B','C'}, {'D','E','F'},
		  {'G','H','I'}, {'J','K','L'}, {'M','N','O'},
		  {'P','R','S'}, {'T','U','V'}, {'W','X','Y'}
	  };
  int prior_pressed, times;
  /***************************************/

  /* declare and set variables for Mode4 */
  struct cursor cursor;
  struct argu_mode_cursor argu_cursor;
  argu_cursor.env = &env;
  pthread_t cursor_thread;
  unsigned int cursor_hide = 0;
  unsigned char mask[10] = {0};
  const size_t size_mask = 10*sizeof(unsigned char);
  /***************************************/

  /* TODO: Make "Initialize the board" */

  /********************/


  while (!quit) {
    // event handling
    if (msgrcv(msqid, &rcv_buf, buf_length, MTYPE_READKEY, IPC_NOWAIT) != minus_one) {
      unsigned int code = rcv_buf.mtext[0];

      switch (code) {
      case BACK:
          {
            kill(env.pid_input, SIGINT);
            kill(env.pid_output, SIGINT);
            kill(getpid(), SIGINT);
          } break;
      case VOL_P: 
          {
            ++ mode; mode &= 0x03;
            need_init = TRUE;
          } break;
      case VOL_M:
          {
            mode += mode_minus; mode &= 0x03;
            need_init = TRUE;
          } break;
      }
      printf("Current Mode: %d\n", mode);
    }


    switch(mode) {
    case 0:
        {
          if (need_init) {
            MSG_SND(DEVICE_CLEAR, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            need_init = FALSE;
            mode_change_time = 0; // TODO: same feature with led flick
            argu_flick.led_flick = &mode_change_time;

            // set led D1
            snd_buf.mtext[1] = 128;
            MSG_SND(ID_LED, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            snd_buf.mtext[1] = cur_hour/10; snd_buf.mtext[2] = cur_hour%10;
            snd_buf.mtext[3] = cur_min/10;  snd_buf.mtext[4] = cur_min%10;
            MSG_SND(ID_FND, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
          }
          // get push_switch
          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one) {
            // Modify time in board
            if (rcv_buf.mtext[0]) {
              mode_change_time ^= 1;
              if (mode_change_time && (pthread_create(&flicker_thread, NULL, &led_flicker, (void*)&argu_flick) != 0)) {
                perror("pthread_create");
                // TODO: kill all processes or just notice error occurunce.
              }
              if (!mode_change_time) {
                usleep(350000);
                snd_buf.mtype = 1;
                snd_buf.mtext[0] = ID_LED;
                snd_buf.mtext[1] = 128;
                if (msgsnd(msqid, &snd_buf, buf_length, IPC_NOWAIT) < 0) {
                  perror("msgsnd from main");
                  // TODO: stop
                }
              }
            }
            // Reset time in board
            else if (rcv_buf.mtext[1]) {
              snd_buf.mtype = 1;
              memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
              snd_buf.mtext[0] = ID_FND;

              if (msgsnd(msqid, &snd_buf, buf_length, IPC_NOWAIT) < 0) {
                perror("msgsnd from main");
                // TODO: stop
              }

            }
            if (mode_change_time) {
              // message get
              if (rcv_buf.mtext[2]) ++cur_hour;
              if (rcv_buf.mtext[3]) ++ cur_min;
              // Correct time
              if (cur_min == 60) {
                cur_min = 0;
                ++ cur_hour;
              }
              cur_hour %= 24;

              // send fnd data
              snd_buf.mtype = 1;
              snd_buf.mtext[0] = ID_FND;
              snd_buf.mtext[1] = cur_hour/10; snd_buf.mtext[2] = cur_hour%10;
              snd_buf.mtext[3] = cur_min/10;  snd_buf.mtext[4] = cur_min%10;

              if (msgsnd(msqid, &snd_buf, buf_length, IPC_NOWAIT) < 0) {
                perror("msgsnd from main");
                // TODO: stop
              } 
            }
          }
          //          memset(rcv_buf.mtext, 0, 20);
        } break; // End of Mode1


    case 1:
        {
          if (need_init) {
            MSG_SND(DEVICE_CLEAR, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            need_init = FALSE;
            idx_base = 0, count = 0;

            snd_buf.mtext[1] = led_num[0];
            MSG_SND(ID_LED, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
            memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
            MSG_SND(ID_FND, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
          }

          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one) {
            if (rcv_buf.mtext[0]) {
              // change base
              ++idx_base;
              idx_base &= 0x03;

              snd_buf.mtext[1] = led_num[idx_base];
              MSG_SND(ID_LED, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
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
              snd_buf.mtext[1] = 0;
            }
            MSG_SND(ID_FND, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
          }
        } break; // End of Mode 2


    case 2:
        {
           if (need_init) {
            MSG_SND(DEVICE_CLEAR, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            need_init = FALSE;
            text_mode = TRUE;
            count = 0, idx_text = -1;

            snd_buf.mtext[1] = 32; // Set D3 led
            MSG_SND(ID_LED, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            memset(text, 0, sizeof(text));
            memcpy(snd_buf.mtext+1, text, MAX_TEXT);
            MSG_SND(ID_LCD, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            memset(snd_buf.mtext, 0, sizeof(snd_buf.mtext));
            MSG_SND(ID_FND, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            memcpy(snd_buf.mtext+1, fpga_alpha, sizeof(fpga_alpha));
            MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
// DEBUG            printf("buf_length: %zu, sizeof: %zu\n", buf_length, sizeof(message_buf));
          }

          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, 11, IPC_NOWAIT) != minus_one) {
// DEBUG            printf("2 buf_length: %zu, sizeof: %zu\n", buf_length, sizeof(message_buf));
            if (rcv_buf.mtext[1] && rcv_buf.mtext[2]) {
              memset(text, 0, sizeof(text));
              idx_text = -1;
            }
            else if (rcv_buf.mtext[4] && rcv_buf.mtext[5]) {
              text_mode ^= 1;

              int str_size = sizeof(fpga_alpha);
              if (text_mode) {
                memcpy(snd_buf.mtext+1, fpga_alpha, 10*sizeof(char));
                MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
              }
              else {
                memcpy(snd_buf.mtext+1, fpga_number[1], 10*sizeof(char));
                MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
              }
            }
            else if (rcv_buf.mtext[7] && rcv_buf.mtext[8]) {
              if (idx_text != LIMIT_TEXT) ++ idx_text;
              else SHIFT_TEXT(text);
              text[idx_text] = ' ';
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
                  break;
                }
              } while(i--);
            }

            memcpy(snd_buf.mtext+1, text, MAX_TEXT);
            MSG_SND(ID_LCD, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
          }
        } break; // End of Mode 3


    case 3:
        {
          if (need_init) {
            MSG_SND(DEVICE_CLEAR, msqid, snd_buf, buf_length, MTYPE_OUTPUT);

            need_init = FALSE;
            memset(snd_buf.mtext+1, 0x00, size_mask);
            MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
            argu_cursor.cursor = &cursor;
            argu_cursor.mask = snd_buf.mtext+1;
            argu_cursor.mode = &mode;
            argu_cursor.cursor_hide = &cursor_hide;
            cursor.x = 1, cursor.y = 0;
            
            if (pthread_create(&cursor_thread, NULL, &print_cursor, (void*)&argu_cursor) != 0) {
              printf("fail: pthread_create\n");
              return -1;
            }
          }
          if (msgrcv(msqid, &rcv_buf, MAX_MSGSZ, MTYPE_SWITCH, IPC_NOWAIT) != minus_one) {
            /* Move the cursor */
            if(rcv_buf.mtext[1] && cursor.y)        -- cursor.y;
            if(rcv_buf.mtext[3] && cursor.x > 1)    -- cursor.x;
            if(rcv_buf.mtext[5] && !(cursor.x > 7)) ++ cursor.x;
            if(rcv_buf.mtext[7] && !(cursor.y > 9)) ++ cursor.y;

            /* Modify the board setting */
            if (rcv_buf.mtext[0]) {
              cursor.x = 1, cursor.y = 0;
              memset(snd_buf.mtext + 1, 0x00, sizeof(mask));
              MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
            }
            if (rcv_buf.mtext[2]) {
              cursor_hide ^= 1;
            }
            if (rcv_buf.mtext[4]) {
              // select and toggle the point
              snd_buf.mtext[cursor.y+1] ^= (0x80 >> cursor.x);
              MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
            }
            if (rcv_buf.mtext[6]) {
              memset(snd_buf.mtext+1, 0x00, sizeof(mask));
              MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
            }
            if (rcv_buf.mtext[8]) {
              // Invert the board
              int i = 10;
              do {
                snd_buf.mtext[i] ^= 0xFF;
              } while(--i);
              MSG_SND(ID_DOT, msqid, snd_buf, buf_length, MTYPE_OUTPUT);
            }
          }
        } break; // End of Mode 4


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
  return 0;
}
