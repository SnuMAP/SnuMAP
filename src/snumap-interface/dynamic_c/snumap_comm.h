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

#include <sys/ipc.h>
#include <sys/shm.h>

#include <dlfcn.h>
#include <errno.h>
//#include "uthash.h"

#define DEBUG_MODE    ///< debug mode shows debug messages

// ioctl definition
#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)
#define IOCTL_GET_JIFFIES          _IOR(MAJOR_NUM, 4, NULL)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

FILE* call_info;
int call_func_id;

int is_started;
int is_opened;
int fd;

/* library implementation */
int init_profiling(void);
int start_profiling(void);
int stop_profiling(void);
int dump_profile_result(void);
int cleanup_profiling(void);
int read_jiffies(volatile unsigned long* jiffies);

/* main initialization routine interpositioning */
int __libc_start_main(int (*main) (int, char **, char **),
    int argc,
    char ** ubp_av,
    void (*init) (void),
    void (*fini) (void),
    void (*rtld_fini) (void),
    void (*stack_end));

/* exit call interpositioning */
void exit(int status);

// @brief GOMP parallel start point interpositioning
void GOMP_parallel_start(void (*fn) (void *), void *data, unsigned num_threads);

/// @brief GOMP parallel end point interpositioning
void GOMP_parallel_end(void);

/// GOMP parallel region
void GOMP_parallel (void (*fn) (void *), void *data, unsigned num_threads, unsigned int flags);

#endif // __SNUMAP_COMM_H

