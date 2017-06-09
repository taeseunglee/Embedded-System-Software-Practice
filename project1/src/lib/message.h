#ifndef __MESSAGE__
#define __MESSAGE__

#include <sys/types.h>
#include <sys/ipc.h>
#include <sys/msg.h>
#include "./environment.h"

/* # of modes */
#define NUM_MODE 5

// Message size
#define MAX_MSGSZ 64
#define BUFF_SIZE 40 // buffer of message text
#define MAX_BUTTON 9 // # of Push switch buttons

/* message type */
#define MTYPE_READKEY 10
#define MTYPE_SWITCH  11
#define MTYPE_OUTPUT  30

/* structure for Messege queue, IPC */
typedef struct msgbuf {
    long mtype;
    unsigned char mtext[BUFF_SIZE];
} message_buf;


// usage: MSGSND_OR_DIE(msqid, &snd_buf, msgsz, IPC_NOWAIT);
#define MSGSND_OR_DIE(...) \
  if (msgsnd(__VA_ARGS__) < 0) \
    { \
      perror("msgsnd"); \
    }

#define set_out_buf(snd_buf, device)\
  ({ \
   snd_buf.mtype = MTYPE_OUTPUT; \
   snd_buf.mtext[0] = device;\
  })

#endif /* __MESSAGE__ */
