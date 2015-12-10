//------------------------------------------------------------------------------
/// @brief The monitor for querying PMU
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/11 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription monitor.cpp
/// The monitor for querying PMU. This is an entry point for profiling.
///
//------------------------------------------------------------------------------

#include <thread>
#include "common.h"
#include "profiler.h"

#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101
#define IOCTL_GET_JIFFIES _IOR(MAJOR_NUM, 4, NULL)

static int is_opened;
static int fd;

void init_kernel_interface(void)
{
  char buf[4096];
  char* env = getenv("OMP_PROFILER_ROOT");

  if (env == NULL) {
    perror("getenv(OMP_PROFILER_ROOT)");
    abort();
  }

  for (int i = 0; i < 4096; i++)
    buf[i] = 0;

  strncat(buf, env, strlen(env));
  strncat(buf, "/", 1);
  strncat(buf, DEVICE_FILE_NAME, strlen(DEVICE_FILE_NAME));

  fd = open(buf, 0);
  if (fd < 0) {
    fprintf(stderr, "can't open the device file: %s\n", buf);
    exit(-1);
  }

  is_opened = 1;
}

void PeriodicSchedulerRoutine(CProfiler* profiler)
{
  while (true) {
    /* periodically try to perform system-wide profiling */
    usleep(1000); // 1ms - current default sampling duration

    /* stop the profiling: update profiling results */
    profiler->StopProfiling();

    /* read jiffies */
    unsigned long jiffies = 0;
    if (is_opened) {
      if (ioctl(fd, IOCTL_GET_JIFFIES, &jiffies)) {
        fprintf(stderr, "ioctl error\n");
      }
    }

    fprintf(stdout, "jiffies: %lu\n", jiffies);

    /* do something */

    /* start new profiling */
    profiler->StartProfiling();
  }
}

int main(int argc, char *argv[])
{
  /* initialize kernel interface */
  init_kernel_interface();

  /* profiler setting */
  CProfiler *profiler =
    //NULL;
    new CSimpleProfiler();
    //new CAdvancedProfiler();

  /* run periodic profiling */
  std::thread periodicScheduler(PeriodicSchedulerRoutine, profiler);

  periodicScheduler.join();

  return 0;
}

