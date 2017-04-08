#ifndef __MAIN_PROCESS__
#define __MAIN_PROCESS_0_

#include "./process.h"

extern unsigned char fpga_number[10][10];
extern unsigned char fpga_alpha[10];
extern unsigned char fpga_set_full[10];
extern unsigned char fpga_set_blank[10];


int main_process(struct environment *env);

#endif
