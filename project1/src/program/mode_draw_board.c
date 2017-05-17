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
static struct argu_mode_cursor argu_cursor =
  {
    .cursor = &cursor,
    .mask = snd_buf.mtext+1,
    .cursor_hide = &cursor_hide,
  };


/* In mode4, this is called to main process as a start routine of pthread for
 * print cursor at DOT device */
void* print_cursor(void *arguments)
{
  struct argu_mode_cursor *argu = arguments;

  unsigned int * mode = argu->mode,
               * cursor_hide = argu->cursor_hide;
  struct cursor* cursor = argu->cursor;
  unsigned char*mask = argu->mask, res[10] = {0};

  struct environment *env = argu->env;
  int dot_fd = env->dot_fd;

  int i = 0;

  while ((*mode) == 3 && !quit)
    {
      if (*cursor_hide)
        write(dot_fd, mask, LEN_DOT);
      else
        {
          i = 4;
          do {
              memcpy(res, mask, LEN_DOT);
              res[cursor->y] ^= (0x80>>(cursor->x));
              write(dot_fd, res, LEN_DOT);
              usleep(250000);
          } while(--i);
        }

      if ((*mode) != 3 || quit)
        break;

      // hide
      i = 4;
      do {
          write(dot_fd, res, LEN_DOT);
          usleep(250000);
      } while(--i);
    }
  memset(mask, 0, LEN_DOT);

  pthread_exit(NULL);
}

void
mode_draw_board_global_init(struct environment *__env, int __msqid)
{
  env   = __env;
  msqid = __msqid;

  argu_cursor.env = env;
  argu_cursor.mode = &(env->mode);
}

void
mode_draw_board_init()
{
  set_out_buf(snd_buf, DEVICE_CLEAR);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  snd_buf.mtext[1] = 16; // Set D4 led
  set_out_buf(snd_buf, ID_LED);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  memset(snd_buf.mtext+1, 0, size_mask);
  set_out_buf(snd_buf, ID_DOT);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);

  cursor.x = 0, cursor.y = 0;

  count = 0;
  if (pthread_create(&cursor_thread, NULL, &print_cursor, (void*)&argu_cursor) != 0)
    perror("pthread_create");
}

void
mode_draw_board(message_buf rcv_buf)
{
  /* Move the cursor */
  if(rcv_buf.mtext[1])
    {
      if (cursor.y)
        -- cursor.y;
      ++ count;
    }
  if(rcv_buf.mtext[3])
    {
      if (cursor.x) 
        -- cursor.x;
      ++ count;
    }
  if(rcv_buf.mtext[5])
    {
      if (!(cursor.x > 6))
        ++ cursor.x;
      ++ count;
    }
  if(rcv_buf.mtext[7])
    {
      if (!(cursor.y > 9))
        ++ cursor.y;
      ++ count;
    }

  /* Modify the board setting */
  if (rcv_buf.mtext[0])
    {
      cursor.x = 0, cursor.y = 0;
      memset(snd_buf.mtext+1, 0x00, sizeof(mask));

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
      snd_buf.mtext[cursor.y+1] ^= (0x40 >> cursor.x);
      ++ count;
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
  if (rcv_buf.mtext[6])
    {
      memset(snd_buf.mtext+1, 0x00, sizeof(mask));
      ++ count;
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }
  if (rcv_buf.mtext[8])
    {
      // Invert the board
      int i = 10;
      do {
        snd_buf.mtext[i] ^= 0xFF;
      } while(--i);
      ++ count;
      set_out_buf(snd_buf, ID_DOT);
      MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
    }

  // TODO: Use mask!! --> change "Use directly snd_buf.mtext" to "Use mask as a field and copy this to mtext"
  memcpy(mask, snd_buf.mtext+1, size_mask);
  snd_buf.mtext[1] = (count/1000)%10; snd_buf.mtext[2] = (count/100)%10;
  snd_buf.mtext[3] = (count/10)%10;  snd_buf.mtext[4] = count%10;
  set_out_buf(snd_buf, ID_FND);
  MSGSND_OR_DIE(msqid, &snd_buf, buf_length, IPC_NOWAIT);
  memcpy(snd_buf.mtext+1, mask, size_mask);

  count %= 10000;
}
