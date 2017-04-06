#include "../src/environment.h"

struct environment*
construct_environment(struct environment* env)
{
  if (env) {
    perror("Env is Not NULL");
    exit(1);
  }

  env = calloc(1, sizeof(struct environment));

  /* Open Device and Check whether an error occurs */
  /* devices for reading data */
  env->ev_fd = open(EVENT_DEVICE, O_RDWR | O_NONBLOCK);
  DETECT_DEVICE_ERROR(env->ev_fd, EVENT_DEVICE);
  env->push_switch_fd = open(PUSH_SWITCH_DEVICE, O_RDWR | O_NONBLOCK);
  DETECT_DEVICE_ERROR(env->push_switch_fd, PUSH_SWITCH_DEVICE);

  /* devices for writing data */
  env->fnd_fd = open(FND_DEVICE, O_RDWR);
  DETECT_DEVICE_ERROR(env->fnd_fd, FND_DEVICE);
  env->led_fd = open(LED_DEVICE, O_RDWR);
  DETECT_DEVICE_ERROR(env->led_fd, LED_DEVICE);
  env->dot_fd = open(FPGA_DOT_DEVICE, O_WRONLY);
  DETECT_DEVICE_ERROR(env->dot_fd, FPGA_DOT_DEVICE);
  env->lcd_fd = open(FPGA_TEXT_LCD_DEVICE, O_WRONLY);
  DETECT_DEVICE_ERROR(env->lcd_fd, FPGA_TEXT_LCD_DEVICE);

  if (signal(SIGINT, quit_signal) == SIG_ERR)
    printf("\ncan't catch SIGINT\n");
}

void
destruct_environment(struct environment* env)
{
  close(env->ev_fd);
  close(env->push_switch_fd);
  close(env->fnd_fd);
  close(env->led_fd);
  close(env->lcd_fd);
  close(env->dot_fd);
}


