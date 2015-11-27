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

void PeriodicSchedulerRoutine(CProfiler* profiler)
{
  while (true) {
    /* periodically try to perform system-wide profiling */
    usleep(1000); // 1ms - current default sampling duration

    /* stop the profiling: update profiling results */
    profiler->StopProfiling();

    /* do something */

    /* start new profiling */
    profiler->StartProfiling();
  }
}

int main(int argc, char *argv[])
{
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

