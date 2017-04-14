#include "../process/process.h"
#include "../thread/cursor_thread.h"

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

