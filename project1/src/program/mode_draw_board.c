#include "../process/process.h"
#include "./mode_draw_board.h"

/* Variablefor message queue */
static int msqid;
static message_buf snd_buf;
static const size_t buf_length = sizeof(message_buf);
static struct environment *env;

static struct cursor cursor;
static pthread_t cursor_thread;
static unsigned int cursor_hide, count;
static unsigned char mask[10];
static const size_t size_mask = sizeof(mask);
static int is_mode_running = FALSE;
static int is_cursor_moved = FALSE;


void
mode_draw_board_global_init(struct environment *__env, int __msqid)
{
  env   = __env;
  msqid = __msqid;
}


void
mode_draw_board_init()
{
  /* init variable */
  is_mode_running = TRUE;
  is_cursor_moved = FALSE;
  cursor.x = 0, cursor.y = 0;
  cursor_hide = 1;
  count = 0;
  memset(mask, 0x00, size_mask);

  /* init devices */
  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  snd_buf.mtext[1] = 16; // Set D4 led
  set_out_buf(snd_buf, ID_LED);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  memset(snd_buf.mtext+1, 0, size_mask);
  set_out_buf(snd_buf, ID_DOT);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  /* make pthread with beginning of mode */
  if (pthread_create(&cursor_thread, NULL, &print_cursor, NULL) != 0)
    perror("pthread_create");
}

void
mode_draw_board_exit()
{
  is_mode_running = FALSE;
  pthread_join(cursor_thread, NULL);

  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
}


/* ------------------------------------------------------------------------ */
/* In mode4, this is called to main process as a start routine of pthread for
 * print cursor at DOT device */
void* print_cursor(void *arguments UNUSED)
{
  unsigned char res[10] = {0};
  int dot_fd = env->dot_fd;

  int i = 0;

  printf("print_cursor on\n");
  while (is_mode_running)
    {
      if (!cursor_hide)
        {
          i = 4;
          do
            {
              memcpy(res, mask, LEN_DOT);
              res[cursor.y] ^= (0x40>>(cursor.x));
              write(dot_fd, res, LEN_DOT);
              usleep(250000);
            } while(--i && is_mode_running);
        }

      if (!is_mode_running)
        break;

      // hide
      i = 5;
      write(dot_fd, res, LEN_DOT);
      do
        {
          usleep(200000);
        } while(--i && (is_mode_running & !is_cursor_moved));
      is_cursor_moved = FALSE;
    }

  printf("print_cursor off\n");
  pthread_exit(NULL);
}

void
mode_draw_board(message_buf rcv_buf)
{
  /* Move the cursor */
  // up
  if(rcv_buf.mtext[1])
    {
      if (cursor.y) {
        -- cursor.y;
        is_cursor_moved = TRUE;
      }
      ++ count;
    }
  // left
  if(rcv_buf.mtext[3])
    {
      if (cursor.x) {
        -- cursor.x;
        is_cursor_moved = TRUE;
      }
      ++ count;
    }
  // right
  if(rcv_buf.mtext[5])
    {
      if (!(cursor.x > 6)) {
        ++ cursor.x;
        is_cursor_moved = TRUE;
      }
      ++ count;
    }
  // down
  if(rcv_buf.mtext[7])
    {
      if (!(cursor.y > 9)) {
        ++ cursor.y;
        is_cursor_moved = TRUE;
      }
      ++ count;
    }

  /* Modify the board setting */
  if (rcv_buf.mtext[0])
    {
      cursor.x = 0, cursor.y = 0;
      memset(mask, 0x00, size_mask);
      memset(snd_buf.mtext+1, 0x00, size_mask);

      ++ count;
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }

  if (rcv_buf.mtext[2])
    {
      cursor_hide ^= 1;
      ++ count;
    }
  
  if (rcv_buf.mtext[4])
    {
      // select and toggle the point
      ++ count;
      mask[cursor.y] ^= (0x40 >> cursor.x);
      memcpy(snd_buf.mtext+1, mask, size_mask);
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
  if (rcv_buf.mtext[6])
    {
      ++ count;
      memset(mask, 0x00, size_mask);
      memset(snd_buf.mtext+1, 0x00, size_mask);
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
  if (rcv_buf.mtext[8])
    {
      // Invert the board
      int i = 10;
      do 
        {
          snd_buf.mtext[i] ^= 0xFF;
        } while(--i);
      ++ count;
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }

  snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
  snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  count %= 10000;
}
