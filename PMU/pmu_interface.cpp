//------------------------------------------------------------------------------
/// @brief PMU interface protocol
/// @author Eunjin Song <eunjin@csap.snu.ac.kr> and
///         Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/02 Eunjin Song   implement PMUReqManager
/// 2015/07 Younghyun Cho code refactoring
/// 2015/11 Younghyun Cho apply to trace-profiler
///
/// @section license_section Licence
/// Copyright (c) 2014,2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription pmu_interface.cpp
/// PMU interface protocol implementation
/// 
//------------------------------------------------------------------------------

#include <cassert>
#include <cstring>
#include <cstdio>
#include <sys/ioctl.h>
#include <unistd.h>

#include "pmu_interface.h"

#ifndef __NR_perf_event_open
#if defined(__PPC__)
#define __NR_perf_event_open	319
#elif defined(__i386__)
#define __NR_perf_event_open	336
#elif defined(__x86_64__)
#define __NR_perf_event_open	298
#elif defined(__tile__)
#define __NR_perf_event_open	241
#else
#error __NR_perf_event_open must be defined
#endif
#endif

#define PRINT_COUNT	1

struct read_format {
  unsigned long long value;         /* The value of the event */
  unsigned long long time_enabled;  /* if PERF_FORMAT_TOTAL_TIME_ENABLED */
};

PMUReqManager::PMUReqManager()
{
  memset(_fd, 0, sizeof(_fd)) ;
  InitPerfEventAttr();
}

PMUReqManager::~PMUReqManager()
{
}

void PMUReqManager::InitPerfEventAttr()
{
  memset(&_attr, 0, sizeof(_attr));

  for (int i = 0; i < MAX_EVENTS; i++) {
    // default attribute configuration
    _attr[i].inherit = 1;
    _attr[i].disabled = 1;
    _attr[i].size = sizeof(_attr[i]);
    _attr[i].read_format = PERF_FORMAT_TOTAL_TIME_ENABLED;
    _attr[i].type = TOTAL_CYCLES_TYPE;
    _attr[i].config = TOTAL_CYCLES_CONFIG;
  }

  for (int i = 0; i < MAX_EVENTS; i++) {
    // default event type
    _event_type[i] = PER_CORE;
  }
}

void PMUReqManager::InitPerfEventAttr(
    std::pair<__u32, __u64>* attr_mapping,
    event_type* attr_type,
    int num_events)
{
  if (num_events > MAX_EVENTS) {
    std::cout << "maximum number of events: " << MAX_EVENTS
      << " can't register " << num_events << " events." << std::endl;
    return;
  }

  for (int i = 0; i < num_events; i++) {
    // attribute configuration
    _attr[i].inherit = 1;
    _attr[i].disabled = 1;
    _attr[i].size = sizeof(_attr[i]);
    _attr[i].read_format = PERF_FORMAT_TOTAL_TIME_ENABLED;
    _attr[i].type = attr_mapping[i].first;
    _attr[i].config = attr_mapping[i].second;
  }

  for (int i = 0; i < num_events; i++) {
    // event type configuration
    _event_type[i] = attr_type[i];
  }
}

void PMUReqManager::GetCurrentCount(event_values& val)
{
  std::lock_guard<std::mutex>  lock(_cur_mutex);
  val = _cur;
}

void PMUReqManager::OpenEventFDs()
{
  for (int j = 0; j < MAX_EVENTS; j++) {
    for (int i = 0; i < NUM_CORES; i++) {
      if (_event_type[j] == PER_CLUSTER &&
          (i % NUM_CORES_IN_CLUSTER != 0)) {
        _fd[j][i] = _fd[j][i - (i % NUM_CORES_IN_CLUSTER)];
        continue;
      }

      // perf_event_open(perf_event_attr, pid, cpuid, is_group, flag);
      _fd[j][i] = syscall(__NR_perf_event_open, &_attr[j], -1, i, -1, 0);
      if (_fd[j][i] < 0) {
        perror("sys_perf_event_open");
        abort();
      }

      ioctl(_fd[j][i], PERF_EVENT_IOC_ENABLE);
    }
  }
}

void PMUReqManager::CloseEventFDs() {
  for (int j = 0; j < MAX_EVENTS; j++) {
    for (int i = 0; i < NUM_CORES; i++) {
      if (_event_type[j] == PER_CLUSTER &&
          (i % NUM_CORES_IN_CLUSTER != 0)) {
        continue;
      }

      ioctl(_fd[j][i], PERF_EVENT_IOC_DISABLE);
      close(_fd[j][i]);
    }
  }
}

void PMUReqManager::OpenEventFDs(int subset_index)
{
  for (int j = subset_index; j < MAX_EVENTS; j++) {
    for (int i = 0; i < NUM_CORES; i++) {
      if (_event_type[j] == PER_CLUSTER &&
          (i % NUM_CORES_IN_CLUSTER != 0)) {
        _fd[j][i] = _fd[j][i - (i % NUM_CORES_IN_CLUSTER)];
        continue;
      }

      // perf_event_open(perf_event_attr, pid, cpuid, is_group, flag);
      _fd[j][i] = syscall(__NR_perf_event_open, &_attr[j], -1, i, -1, 0);
      if (_fd[j][i] < 0) {
        perror("sys_perf_event_open");
        abort();
      }

      ioctl(_fd[j][i], PERF_EVENT_IOC_ENABLE);
    }
  }
}

void PMUReqManager::CloseEventFDs(int subset_index)
{
  for (int j = subset_index; j < MAX_EVENTS; j++) {
    for (int i = 0; i < NUM_CORES; i++) {
      if (_event_type[j] == PER_CLUSTER &&
          (i % NUM_CORES_IN_CLUSTER != 0)) {
        continue;
      }

      ioctl(_fd[j][i], PERF_EVENT_IOC_DISABLE);
      close(_fd[j][i]);
    }
  }
}

void PMUReqManager::RecordAllCounters() {
  std::lock_guard<std::mutex>  lock(_cur_mutex);
  ReadCounters(&_cur);
}

void PMUReqManager::ReadCounters(event_values *values)
{
  struct read_format count_result;
  unsigned long long count = 0;

  for (int j = 0; j < MAX_EVENTS; j++) {
    for (int i = 0; i < NUM_CORES; i++) {
      if (_event_type[j] == PER_CLUSTER && (i % NUM_CORES_IN_CLUSTER != 0)) {
        values->values[j][i] = 0;
        //values->values[j][i] = values->values[j][i-(i % NUM_CORES_IN_CLUSTER)];
        continue;
      }

      size_t res;
      res = read(_fd[j][i], &count_result, sizeof(struct read_format));
      if (res != sizeof(struct read_format)) {
        perror("read file descriptor error");
        abort();
      }

      values->values[j][i] = count_result.value;
      values->timestamp[j][i] = count_result.time_enabled;
#if PRINT_COUNT
      std::cout << "event: " << j << " cpu: " << i
        << " count: " << values->values[j][i] << std::endl;
#endif
    }
  }
}

