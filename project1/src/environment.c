#include "../src/environment.h"
#include <stdio.h>
#include <errno.h>

#define DETECT_DEVICE_ERROR(dev_fd, DEVICE) \
 if (dev_fd < 0) { \
   printf("Device open error : %s\n", DEVICE); \
   exit(1); \
 }

// construct_environment must be called by main function
// when program begins
void
construct_environment(struct environment** env)
{
  struct environment *new_env = calloc(1, sizeof(struct environment));

  /* Open Device and Check whether an error occurs */
  /* devices for reading data */
  new_env->ev_fd = open(EVENT_DEVICE, O_RDWR | O_NONBLOCK);
  DETECT_DEVICE_ERROR(new_env->ev_fd, EVENT_DEVICE);
  new_env->push_switch_fd = open(PUSH_SWITCH_DEVICE, O_RDWR | O_NONBLOCK);
  DETECT_DEVICE_ERROR(new_env->push_switch_fd, PUSH_SWITCH_DEVICE);

  /* devices for writing data */
  new_env->fnd_fd = open(FND_DEVICE, O_RDWR);
  DETECT_DEVICE_ERROR(new_env->fnd_fd, FND_DEVICE);
  new_env->led_fd = open(LED_DEVICE, O_RDWR);
  DETECT_DEVICE_ERROR(new_env->led_fd, LED_DEVICE);
  new_env->dot_fd = open(FPGA_DOT_DEVICE, O_WRONLY);
  DETECT_DEVICE_ERROR(new_env->dot_fd, FPGA_DOT_DEVICE);
  new_env->lcd_fd = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
  DETECT_DEVICE_ERROR(new_env->lcd_fd, FPGA_TEXT_LCD_DEVICE);

  if (signal(SIGINT, quit_signal) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");

  /* Setting the key of message queue */
  new_env->msg_key = 1234;

  new_env->mode = 0;

  *env = new_env;

  printf("Construct Environment\n");
}


// destruct_environment must be called by main process
// when input and output process are terminated
void
destruct_environment(struct environment* env)
{
#define CLOSE_ERROR_CHECK(fd) \
  if (close(fd) == -1) { \
      printf("Errno: %d\n", errno); \
      perror("close"); \
  }

  CLOSE_ERROR_CHECK(env->ev_fd);
  CLOSE_ERROR_CHECK(env->push_switch_fd);
  CLOSE_ERROR_CHECK(env->fnd_fd);
  CLOSE_ERROR_CHECK(env->led_fd);
  CLOSE_ERROR_CHECK(env->lcd_fd);
  CLOSE_ERROR_CHECK(env->dot_fd);

#undef CLOSE_ERROR_CHECK

  free(env);

  printf("Destruct Envrionment\n");
}

void kill_all_processes(struct environment *env)
{
  kill(env->pid_input, SIGINT);
  kill(env->pid_output, SIGINT);
  kill(env->pid_main, SIGINT);
}

void quit_signal(int sig)
{
  quit = 1;
}
