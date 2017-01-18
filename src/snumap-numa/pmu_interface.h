//------------------------------------------------------------------------------
/// PMU interface for NUMA Performance Profiler
/// 

#ifndef __PMU_INTERFACE_H_
#define __PMU_INTERFACE_H_

#include <cstdlib>
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/epoll.h>

#include <vector>
#include "numa_event.h"

class PMUReqManager {
  public:
    PMUReqManager(int num_cores, int cores_in_node, int num_events);
    ~PMUReqManager();

    void InitPerfEventAttr(std::pair<__u32, __u64>* attr_mapping,
        event_type* attr_type);
    void InitPerfEventAttr(int core_num,
        std::pair<__u32, __u64>* attr_mapping,
        event_type* attr_type);
    void GetCurrentCount(sc_event_values& val);
    void OpenEventFDs();
    void OpenEventFDs(int subset_index);
    void CloseEventFDs();
    void CloseEventFDs(int subset_index);
    void RecordAllCounters();

  private:
    void ReadCounters(sc_event_values *values);

    int fd_[MAX_EVENTS][NUM_CORES];
    struct perf_event_attr attr_[NUM_CORES][MAX_EVENTS];
    event_type event_type_[MAX_EVENTS];
    sc_event_values cur_;

    const int num_cores_;
    const int cores_in_node_;
    const int num_events_;
};

#endif

