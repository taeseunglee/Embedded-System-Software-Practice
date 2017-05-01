#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>

#ifdef linux
#include <linux/compiler.h>
#endif


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


struct converted_argv
{
  int time_interval;
  int number;
  int startup_option;
};

static inline int
check_and_atoi (const char *str IN);

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
      printf("Usage : integer[1-100] integer[1-100] integer[0001-8000]\n");
      printf("Note : time_interval number_of_times startup_option\n");
      return -1;
    }

  timer_fd = open("/dev/dev_driver", O_WRONLY);
  if (timer_fd < 0)
    {
      printf("Timer Device Open Failured!\n");
      return -1;
    }


  write(timer_fd, &get_number, sizeof(get_number) );
  close(timer_fd);

  return 0;
}

static inline int
check_and_atoi (const char *str IN)
{
  int i, tmp, res;

  if (unlikely(str == NULL))
    {
      printf("[check_out_atoi] String empty Error!\n");
      return -1;
    }

  for (i = 0; str[i]; ++i)
    {
      tmp = str[i] - 0x30;
      if (unlikely(tmp < 0 || tmp > 9))
        {
          perror("[check_out_atoi] Arguments Error\n");
          return -1;
        }
      
      res = ((res<<1) + (res<<3)) + tmp; // res = res*10 + tmp;
    }

  return res;
}

static inline struct converted_argv
check_argv(char **argv IN)
{
  /* Declaration */
  struct converted_argv conv_argv;

  conv_argv.time_interval  = check_and_atoi(argv[1]);
  conv_argv.number         = check_and_atoi(argv[2]);
  conv_argv.startup_option = check_and_atoi(argv[3]);

  return conv_argv;
}

