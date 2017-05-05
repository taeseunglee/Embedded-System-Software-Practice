#ifndef __DEVICE_H__
#define __DEVICE_H__

#define EVENT_DEVICE	      "/dev/input/event0"
#define FND_DEVICE	      "/dev/fpga_fnd"
#define LED_DEVICE	      "/dev/fpga_led"
#define PUSH_SWITCH_DEVICE    "/dev/fpga_push_switch"
#define FPGA_DOT_DEVICE       "/dev/fpga_dot"
#define FPGA_TEXT_LCD_DEVICE  "/dev/fpga_text_lcd"

/* Device IDs */
#define ID_LED    0
#define ID_FND    1
#define ID_DOT    2
#define ID_PS_SW  3
#define ID_LCD    4
#define ID_MOTOR  5
#define ID_DIP_SW 6
#define ID_BUZZER 7

#define DEVICE_CLEAR 30

/* Readkey define */
#define KEY_RELEASE 0
#define KEY_PRESS 1

#define BACK  158
#define VOL_P 115
#define VOL_M 114

#define LEN_LED 1
#define LEN_FND 4
#define LEN_LCD 32
#define LEN_DOT 10

#endif
