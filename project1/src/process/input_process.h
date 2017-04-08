#ifndef __INPUT_PROCESS__
#define __INPUT_PROCESS__

#include "./process.h"

extern unsigned char fpga_number[10][10];
extern unsigned char fpga_alpha[10];
extern unsigned char fpga_set_full[10];
extern unsigned char fpga_set_blank[10];

void input_process(struct environment *env);

#endif
