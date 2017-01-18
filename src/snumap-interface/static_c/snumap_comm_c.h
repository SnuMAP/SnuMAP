//------------------------------------------------------------------------------
/// SnuMAP OpenMP (C/C++ applications) profiler library header
/// 

#ifndef __RTE_COMM_H_
#define __RTE_COMM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <omp.h>

// ioctl definition
#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

int is_opened;
int fd;

/// @name library implementation
/// @{

int omp_init_profiling(void);
int omp_start_profiling(void);
int omp_stop_profiling(void);
int omp_dump_profile_result(void);
int omp_cleanup_profiling(void);

/// @}

#endif // __RTE_COMM_H_
