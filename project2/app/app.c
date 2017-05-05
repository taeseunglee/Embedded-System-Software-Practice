#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#include <sys/ioctl.h>
#include "../module/kernel_timer.h"

#ifndef likely
# define likely(x) __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
# define unlikely(x) __builtin_expect(!!(x), 0)
#endif

#define IN 
#define OUT 
#define INOUT 

#define TRUE  1
#define FALSE 0

/*
struct converted_argv
{
  int time_interval;
  int number;
  int startup_option;
};
*/
static inline int
atoi_and_check (const char *str IN);

static inline struct converted_argv
check_argv(char **argv IN);


int
main(int argc, char **argv)
{
  int timer_fd;
  char get_number;

  /* Arguments checking */
  if(argc != 4)
    {
      printf("Usage : [1-100] [1-100] [0001-8000]\n"
             "Note : time_interval(integer)[1-100]\n"
             "\tnumber_of_times(integer)[1-100]\n"
             "\tstartup_option(integer)[0001-8000]\n");
      printf("Note : time_interval number_of_times startup_option\n");
      return -1;
    }

  timer_fd = open("/dev/dev_driver", O_WRONLY);
  if (timer_fd < 0)
    {
      printf("Timer Device Open Failured!\n");
      return -1;
    }

  long time_interval, number, startup_option;
  long packed_data;
  
  time_interval  = atoi_and_check(argv[1]);
  number         = atoi_and_check(argv[2]);
  startup_option = atoi_and_check(argv[3]);

  printf("param : %ld %ld %ld\n", time_interval, number, startup_option);

  packed_data = syscall(376, time_interval, number, startup_option);

  printf("%ld ", (packed_data & 0xFF000000) >> 24);
  printf("%ld ", (packed_data & 0x00FF0000) >> 16);
  printf("%ld ", (packed_data & 0x0000FF00) >> 8);
  printf("%ld\n", packed_data & 0x000000FF);

  ioctl(timer_fd, KTIMER_START, packed_data);

  close(timer_fd);

  return 0;
}

static inline int
atoi_and_check (const char *num IN)
{
  int i, tmp, res = 0;

  if (unlikely(num == NULL))
    {
      printf("[atoi_after_check] String empty Error!\n");
      return -1;
    }

  for (i = 0; num[i]; ++i)
    {
      tmp = num[i] - 0x30;
      if (unlikely(tmp < 0 || tmp > 9))
        {
          perror("[check_out_atoi] Arguments Error\n");
          return -1;
        }
      
      res = ((res<<1) + (res<<3)) + tmp; // res = res*10 + tmp;
    }

  return res;
}

/*
static inline struct converted_argv
check_argv(char **argv IN)
{
   Declaration
  struct converted_argv conv_argv;

  conv_argv.time_interval  = atoi_and_check(argv[1]);
  conv_argv.number         = atoi_and_check(argv[2]);
  conv_argv.startup_option = atoi_and_check(argv[3]);

  return conv_argv;
}
*/
