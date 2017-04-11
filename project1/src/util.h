#ifndef __UTILS_H__
#define __UTILS_H__

#include "../src/environment.h"
#include "../lib/define.h"
#include <stdio.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/msg.h>
#include <sys/ipc.h>
#include <pthread.h>
#include <string.h>

#define SHIFT_TEXT(text) \
  do { \
	int t=0; \
	while (t<LIMIT_TEXT) { text[t] = text[t+1]; t++; } \
  } while(0);


// TODO: move device_clear to output process
void  device_clear(struct environment *env);

#endif
