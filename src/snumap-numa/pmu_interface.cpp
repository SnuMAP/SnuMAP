//------------------------------------------------------------------------------
/// PMU interface for NUMA Performance Profiler
/// 

#include <cassert>
#include <cstdio>
#include <cstring>
#include <sys/ioctl.h>
#include <unistd.h>

#include "pmu_interface.h"

#ifndef __NR_perf_event_open
#if defined(AMD64)
#define __NR_perf_event_open	298
#elif defined(AMD32)
#define __NR_perf_event_open  298
#endif
#endif

#define PRINT_COUNT	0

struct read_format {
  unsigned long long value;         /* The value of the event */
  unsigned long long time_enabled;  /* if PERF_FORMAT_TOTAL_TIME_ENABLED */
};

PMUReqManager::PMUReqManager(int num_cores, int cores_in_node, int num_events)
  : num_cores_(num_cores), cores_in_node_(cores_in_node), num_events_(num_events)
{
  memset(fd_, 0, sizeof(fd_));
}

PMUReqManager::~PMUReqManager()
{
}

void PMUReqManager::InitPerfEventAttr(std::pair<__u32, __u64>* attr_mapping,
    event_type* attr_type)
{
  for (int i = 0; i < num_cores_; i++) {
    for (int j = 0; j < num_events_; j++) {
      // attribute configuration
      attr_[i][j].inherit = 1;
      attr_[i][j].disabled = 1;
      attr_[i][j].size = sizeof(attr_[i][j]);
      attr_[i][j].read_format = PERF_FORMAT_TOTAL_TIME_ENABLED;
      attr_[i][j].type = attr_mapping[j].first;
      attr_[i][j].config = attr_mapping[j].second;
    }
  }

  for (int i = 0; i < num_events_; i++) {
    // event type configuration
    event_type_[i] = attr_type[i];
  }
}

void PMUReqManager::InitPerfEventAttr(int core_num,
    std::pair<__u32, __u64>* attr_mapping,
    event_type* attr_type)
{
  for (int j = 0; j < num_events_; j++) {
    // attribute configuration
    attr_[core_num][j].inherit = 1;
    attr_[core_num][j].disabled = 1;
    attr_[core_num][j].size = sizeof(attr_[core_num][j]);
    attr_[core_num][j].read_format = PERF_FORMAT_TOTAL_TIME_ENABLED;
    attr_[core_num][j].type = attr_mapping[j].first;
    attr_[core_num][j].config = attr_mapping[j].second;
  }

  for (int i = 0; i < num_events_; i++) {
    event_type_[i] = attr_type[i];
  }
}

void PMUReqManager::GetCurrentCount(sc_event_values& val)
{
  val = cur_;
}

void PMUReqManager::OpenEventFDs()
{
  for (int j = 0; j < num_events_; j++) {
    for (int i = 0; i < num_cores_; i++) {
      if (event_type_[j] == PER_NODE &&
          (i % cores_in_node_ != 0)) {
        fd_[j][i] = fd_[j][i - (i % cores_in_node_)];
        continue;
      }

      //printf("i: %d j: %d num_events: %d (%d) num_cores: %d (%d)\n",
      //        i, j, num_events_, MAX_EVENTS, num_cores_, NUM_CORES);

      // perf_event_open(perf_eventattr_, pid, cpuid, is_group, flag);
      fd_[j][i] = syscall(__NR_perf_event_open, &(attr_[i][j]), -1, i, -1, 0);
      if (fd_[j][i] < 0) {
        perror("sys_perf_event_open");
        abort();
      }

      ioctl(fd_[j][i], PERF_EVENT_IOC_ENABLE);
    }
  }
}

void PMUReqManager::CloseEventFDs() {
  for (int j = 0; j < num_events_; j++) {
    for (int i = 0; i < num_cores_; i++) {
      if (event_type_[j] == PER_NODE &&
          (i % cores_in_node_ != 0)) {
        continue;
      }

      ioctl(fd_[j][i], PERF_EVENT_IOC_DISABLE);
      close(fd_[j][i]);
    }
  }
}

void PMUReqManager::OpenEventFDs(int subset_index)
{
  for (int j = subset_index; j < num_events_; j++) {
    for (int i = 0; i < num_cores_; i++) {
      if (event_type_[j] == PER_NODE &&
          (i % cores_in_node_ != 0)) {
        fd_[j][i] = fd_[j][i - (i % cores_in_node_)];
        continue;
      }

      // perf_event_open(perf_eventattr_, pid, cpuid, is_group, flag);
      fd_[j][i] = syscall(__NR_perf_event_open, &attr_[i][j], -1, i, -1, 0);
      if (fd_[j][i] < 0) {
        perror("sys_perf_event_open");
        abort();
      }

      ioctl(fd_[j][i], PERF_EVENT_IOC_ENABLE);
    }
  }
}

void PMUReqManager::CloseEventFDs(int subset_index)
{
  for (int j = subset_index; j < num_events_; j++) {
    for (int i = 0; i < num_cores_; i++) {
      if (event_type_[j] == PER_NODE &&
          (i % cores_in_node_ != 0)) {
        continue;
      }

      ioctl(fd_[j][i], PERF_EVENT_IOC_DISABLE);
      close(fd_[j][i]);
    }
  }
}

void PMUReqManager::RecordAllCounters() {
  ReadCounters(&cur_);
}

void PMUReqManager::ReadCounters(sc_event_values *values)
{
  struct read_format count_result;
  unsigned long long count = 0;

  for (int j = 0; j < num_events_; j++) {
    for (int i = 0; i < num_cores_; i++) {
      if (event_type_[j] == PER_NODE && (i % cores_in_node_ != 0)) {
        values->values[j][i] = 0;
        //values->values[j][i] = values->values[j][i-(i % cores_in_node_)];
        continue;
      }

      size_t res;
      res = read(fd_[j][i], &count_result, sizeof(struct read_format));
      if (res != sizeof(struct read_format)) {
        perror("read file descriptor error");
        abort();
      }

      values->values[j][i] = count_result.value;
      values->timestamp[j][i] = count_result.time_enabled;
    }
  }
}

