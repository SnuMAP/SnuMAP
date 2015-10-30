//------------------------------------------------------------------------------
/// @brief OpenMP profiler library header
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created
/// 2015/10 Heesik Shin apply IOCTL number macro.
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

// ioctl definition
#include "../Include/ioctldef.h"

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
