#ifndef __DEFINE_H__
#define __DEFINE_H__

/* BOOL */
#define TRUE  1
#define FALSE 0

/* TEXT Macro for mode3 */
#define MAX_TEXT 8
#define LIMIT_TEXT 7  // MAX_TEXT-1

/* # of modes */
#define NUM_MODE 5

// Message size
#define MAX_MSGSZ 64 // TODO: change -> to sizeof(message_buf);
#define BUFF_SIZE 20 // buffer of message text
#define MAX_BUTTON 9 // # of Push switch buttons
#define MAX_DOT   10 // # of charcters for print fpga_dot device

/* message type */
#define MTYPE_READKEY 10
#define MTYPE_SWITCH  11
#define MTYPE_OUTPUT  30

/* structure for Messege queue, IPC */
typedef struct msgbuf {
  long mtype;
  unsigned char mtext[BUFF_SIZE];
} message_buf;


#endif
