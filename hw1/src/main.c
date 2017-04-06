#include "lib/error_check.h"
#include "lib/device.h"
#include "lib/environment.h"
#include "lib/define.h"
#include "src/util.h"

#include <stdio.h>
#include "src/process/process.h"
#include "src/process/main_process.h"
#include "src/process/output_process.h"
#include "src/process/input_process.h"


void input_process(struct environment *env);
void output_process(struct environment *env);

unsigned int quit;
const unsigned int minus_one = -1;

int main()
{
  struct environment *env;
  construct_environment(env);

  // input process
  if (!(env.pid_input = fork())) {
    printf("Start Input process\n");
    input_process(&env);
    exit(0);
  }
  else if (env->pid_input < 0) {
    printf("Input process fork() Error\n");
    return -1;
  }

  // output process
  if (!(env->pid_output = fork())) {
    printf("Start Output process\n");
    output_process(&env);
    exit(0);
  }
  else if (env->pid_output < 0) {
    printf("Output process fork() Error\n");
    return -1;
  }
  env->pid_main = getpid();

  // main process
  printf("Start Main Process\n");
  main_process(&env);

  printf("Main Process exit\n");
  return 0;
}
