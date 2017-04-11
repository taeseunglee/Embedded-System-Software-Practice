#ifndef __MAIN_PROCESS__
#define __MAIN_PROCESS__

#include "./process.h"
#include "../thread/cursor_thread.h"
#include "../thread/led_flicker_thread.h"

extern unsigned char fpga_number[10][10];
extern unsigned char fpga_alpha[10];
extern unsigned char fpga_set_full[10];
extern unsigned char fpga_set_blank[10];


int main_process(struct environment *env);

#endif