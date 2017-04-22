#include "./lib/device.h"
#include "./lib/define.h"
#include "./environment.h"
#include "./util.h"

#include <stdio.h>
#include <unistd.h>
#include "./process/process.h"
#include "./process/main_process.h"
#include "./process/output_process.h"
#include "./process/input_process.h"


unsigned int quit;
const unsigned int minus_one = -1;

extern unsigned char fpga_number[10][10];
extern unsigned char fpga_alpha[10];
extern unsigned char fpga_set_full[10];
extern unsigned char fpga_set_blank[10];

int main()
{
  struct environment *env;
  construct_environment(&env);

  // input process
  if (!(env->pid_input = fork())) {
    printf("Start Input process\n");
    input_process(env);
    exit(0);
  }
  else if (env->pid_input < 0) {
    printf("Input process fork() Error\n");
    return -1;
  }

  // output process
  if (!(env->pid_output = fork())) {
    printf("Start Output process\n");
    output_process(env);
    exit(0);
  }
  else if (env->pid_output < 0) {
    printf("Output process fork() Error\n");
    return -1;
  }
  env->pid_main = getpid();

  // main process
  printf("Start Main Process\n");
  main_process(env);

  destruct_environment(env);
  printf("Main Process exit\n");
  return 0;
}
