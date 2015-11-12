//------------------------------------------------------------------------------
/// @brief OpenMP profiler library header
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2015, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription OMP_prof.h
/// 
//------------------------------------------------------------------------------

#ifndef __RTE_COMM_H_
#define __RTE_COMM_H_

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <omp.h>

struct taskprofile_data {
	int resume_cnt[64];
	int suspend_cnt[64];
	unsigned long resume_time[64][10000];
	unsigned long suspend_time[64][10000];
};

#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)

#define IOCTL_COMMAND_1 IOCTL_START_PROFILING
#define IOCTL_COMMAND_2 IOCTL_STOP_PROFILING
#define IOCTL_COMMAND_3 IOCTL_DUMP_PROFILED_RESULT

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

int is_opened;
int fd;

/// @name library implementation
/// @{

int omp_init_profiling(void);
int omp_start_profiling(void);
int omp_stop_profiling(void);
int omp_dump_proflie_result(void);
int omp_cleanup_profiling(void);

/// @}

#endif // __RTE_COMM_H_
