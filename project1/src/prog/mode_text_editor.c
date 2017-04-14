#include "../../lib/define.h"

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

