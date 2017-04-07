#include "../src/environment.h"

struct environment*
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

  *env = new_env;

  printf("Construct Environment\n");
}


// called this function when output process would be die
void
destruct_environment(struct environment* env)
{
  // TODO: Use return value of close -> error check
  close(env->ev_fd);
  close(env->push_switch_fd);
  close(env->fnd_fd);
  close(env->led_fd);
  close(env->lcd_fd);
  close(env->dot_fd);

  // TODO: free(env);
  // env = NULL

  printf("Destruct Envrionment\n");
}


