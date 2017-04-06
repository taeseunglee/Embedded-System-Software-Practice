#ifndef __ERROR_CHECK_H__
#define __ERROR_CHECK_H__

#include "util.h"

#define DETECT_WRITE_ERROR(ret_bytes, bytes, env) \
 if(ret_bytes != bytes) { \
   printf("Write Error!\n"); \
   kill_all_processes(&env); \
 }
/*
#define DETECT_READ_ERROR(ret_bytes, bytes) \
 if(ret_bytes != bytes) { \
   printf("Read Error!\n"); \
   kill_all_processes(&env); \
 }
*/

#define DETECT_DEVICE_ERROR(dev_fd, DEVICE) \
 if (dev_fd < 0) { \
   printf("Device open error : %s\n", DEVICE); \
   return -1; \
 }

#endif
