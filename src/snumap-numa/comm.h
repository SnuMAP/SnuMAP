#ifndef __SNUMAP_COMM_H
#define __SNUMAP_COMM_H

#define _GNU_SOURCE

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/time.h>
#include <fcntl.h>
#include <string.h>

#include <fcntl.h>
#include <sys/ioctl.h>

#define DEBUG_MODE    ///< debug mode shows debug messages

// ioctl definition
#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)
#define IOCTL_GET_JIFFIES _IOR(MAJOR_NUM, 4, NULL)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

/* library implementation */
int init_profiling(void);
int start_profiling(void);
int stop_profiling(void);
int dump_profile_result(void);
int cleanup_profiling(void);

int read_jiffies(volatile unsigned long* jiffies);

#endif // __SNUMAP_COMM_H

