//------------------------------------------------------------------------------
/// @brief The online performance profiler
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
/// @decription profiler.cpp
/// 
//------------------------------------------------------------------------------

#include "profiler.h"

//------------------------------------------------------------------------------
// CSimpleProfiler
CSimpleProfiler::CSimpleProfiler()
{
  _pmu_manager = new PMUReqManager();

#if defined(AMD64)
  int num_events = 4;
  _attr_mapping = (std::pair<__u32, __u64>*)
    malloc(num_events * sizeof(std::pair<__u32, __u64>));

  _attr_mapping[0] = std::make_pair<__u32, __u64>
    (TOTAL_CYCLES_TYPE, TOTAL_CYCLES_CONFIG);
  _attr_mapping[1] = std::make_pair<__u32, __u64>
    (TOTAL_INSTRUCTIONS_TYPE, TOTAL_INSTRUCTIONS_CONFIG);
  _attr_mapping[2] = std::make_pair<__u32, __u64>
    (STALL_CYCLES_TYPE, STALL_CYCLES_CONFIG);
  _attr_mapping[3] = std::make_pair<__u32, __u64>
    (LLC_MISSES_TYPE, LLC_MISSES_CONFIG);

  _attr_type = (event_type*)malloc(num_events * sizeof(event_type));
  _attr_type[0] = PER_CORE;
  _attr_type[1] = PER_CORE;
  _attr_type[2] = PER_CORE;
  _attr_type[3] = PER_CLUSTER;

#elif defined(AMD32)
  int num_events = 4;
  _attr_mapping = (std::pair<__u32, __u64>*)
    malloc(num_events * sizeof(std::pair<__u32, __u64>));

  _attr_mapping[0] = std::make_pair<__u32, __u64>
    (TOTAL_CYCLES_TYPE, TOTAL_CYCLES_CONFIG);
  _attr_mapping[1] = std::make_pair<__u32, __u64>
    (TOTAL_INSTRUCTIONS_TYPE, TOTAL_INSTRUCTIONS_CONFIG);
  _attr_mapping[2] = std::make_pair<__u32, __u64>
    (STALL_CYCLES_TYPE, STALL_CYCLES_CONFIG);
  _attr_mapping[3] = std::make_pair<__u32, __u64>
    (LLC_MISSES_TYPE, LLC_MISSES_CONFIG);

  _attr_type = (event_type*)malloc(num_events * sizeof(event_type));
  _attr_type[0] = PER_CORE;
  _attr_type[1] = PER_CORE;
  _attr_type[2] = PER_CORE;
  _attr_type[3] = PER_CLUSTER;

#elif defined(TILEGX36)
  int num_events = 5;
  _attr_mapping = (pair<__u32, __u64>*)
    malloc(num_events * sizeof(pair<__u32, __u64>));

  _attr_mapping[0] = std::make_pair<__u32, __u64>
    (TOTAL_CYCLES_TYPE, TOTAL_CYCLES_CONFIG);
  _attr_mapping[1] = std::make_pair<__u32, __u64>
    (LOCAL_DATA_READ_MISS_TYPE, LOCAL_DATA_READ_MISS_CONFIG);
  _attr_mapping[2] = std::make_pair<__u32, __u64>
    (LOCAL_WRITE_MISS_TYPE, LOCAL_WRITE_MISS_CONFIG);
  _attr_mapping[3] = std::make_pair<__u32, __u64>
    (REMOTE_DATA_READ_MISS_TYPE, REMOTE_DATA_READ_MISS_CONFIG);
  _attr_mapping[4] = std::make_pair<__u32, __u64>
    (REMOTE_WRITE_MISS_TYPE, REMOTE_WRITE_MISS_CONFIG);

  _attr_type = (event_type*)malloc(num_events * sizeof(event_type));
  _attr_type[0] = PER_CORE;
  _attr_type[1] = PER_CORE;
  _attr_type[2] = PER_CORE;
  _attr_type[3] = PER_CORE;
  _attr_type[4] = PER_CORE;
#endif

  _pmu_manager->InitPerfEventAttr(_attr_mapping, _attr_type, num_events);
  _pmu_manager->OpenEventFDs();
}

CSimpleProfiler::~CSimpleProfiler()
{
  free(_attr_mapping);
  free(_attr_type);
  _pmu_manager->CloseEventFDs();
  delete _pmu_manager;
}

void CSimpleProfiler::StartProfiling()
{
  _pmu_manager->GetCurrentCount(_start);
}

void CSimpleProfiler::StopProfiling()
{
#if defined(AMD64)
  const int TOTAL_CYCLES = 0;
  const int LLC_MISSES = 3;
#elif defined(AMD32)
  const int TOTAL_CYCLES = 0;
  const int LLC_MISSES = 3;
#elif defined(TILEGX36)
  const int TOTAL_CYCLES = 0;
  const int LOCAL_DATA_READ_MISS = 1;
  const int LOCAL_WRITE_MISS = 2;
  const int REMOTE_DATA_READ_MISS = 3;
  const int REMOTE_WRITE_MISS = 4;
#endif
  event_values _end;

  _pmu_manager->RecordAllCounters();
  _pmu_manager->GetCurrentCount(_end);

  // don't save profiling info. if PMU reading is within given period
  if (_end.values[0][0] == _start.values[0][0]) return;

  for (int core = 0; core < NUM_CORES; core++) {
    unsigned long total_cycles = _end.values[TOTAL_CYCLES][core]
      - _start.values[TOTAL_CYCLES][core];
    unsigned long llc_misses = 0;

#if defined(AMD64)
    if (_attr_type[LLC_MISSES] == PER_CLUSTER) {
      int delegate_core = core - core % NUM_CORES_IN_CLUSTER;
      llc_misses =
        (_end.values[LLC_MISSES][delegate_core] -
         _start.values[LLC_MISSES][delegate_core]);
    } else {
      llc_misses =
        (_end.values[LLC_MISSES][core] - _start.values[LLC_MISSES][core]);
    }
#elif defined(AMD32)
    if (_attr_type[LLC_MISSES] == PER_CLUSTER) {
      int delegate_core = core - core % NUM_CORES_IN_CLUSTER;
      llc_misses =
        (_end.values[LLC_MISSES][delegate_core] -
         _start.values[LLC_MISSES][delegate_core]);
    } else {
      llc_misses =
        (_end.values[LLC_MISSES][core] - _start.values[LLC_MISSES][core]);
    }
#elif defined(TILEGX36)
    llc_misses =
      (_end.values[LOCAL_DATA_READ_MISS][core]
       - _start.values[LOCAL_DATA_READ_MISS][core]);
    llc_misses +=
      (_end.values[LOCAL_WRITE_MISS][core]
       - _start.values[LOCAL_WRITE_MISS][core]);
    llc_misses +=
      (_end.values[REMOTE_DATA_READ_MISS][core]
       - _start.values[REMOTE_DATA_READ_MISS][core]);
    llc_misses +=
      (_end.values[REMOTE_WRITE_MISS][core]
       - _start.values[REMOTE_WRITE_MISS][core]);
#endif

    _sum_llc_misses += llc_misses;
    _sum_total_cycles += total_cycles;

    //std::cout << "core: " << core << " TOT_CYC: " << total_cycles
    //  << " LLC_MISSES: " << llc_misses << std::endl;
  }
}

void CSimpleProfiler::StopProfiling(unsigned long jiffies, ostream & dout)
{
#if defined(AMD64)
  const int TOTAL_CYCLES = 0;
  const int LLC_MISSES = 3;
#elif defined(AMD32)
  const int TOTAL_CYCLES = 0;
  const int TOTAL_INSTRUCTIONS = 1;
  const int STALL_CYCLES = 2;
  const int LLC_MISSES = 3;
#elif defined(TILEGX36)
  const int TOTAL_CYCLES = 0;
  const int LOCAL_DATA_READ_MISS = 1;
  const int LOCAL_WRITE_MISS = 2;
  const int REMOTE_DATA_READ_MISS = 3;
  const int REMOTE_WRITE_MISS = 4;
#endif
  event_values _end;

  _pmu_manager->RecordAllCounters();
  _pmu_manager->GetCurrentCount(_end);

  // don't save profiling info. if PMU reading is within given period
  if (_end.values[0][0] == _start.values[0][0]) return;

  for (int core = 0; core < NUM_CORES; core++) {
    unsigned long total_cycles = _end.values[TOTAL_CYCLES][core]
      - _start.values[TOTAL_CYCLES][core];
    unsigned long total_instructions = _end.values[TOTAL_INSTRUCTIONS][core]
      - _start.values[TOTAL_INSTRUCTIONS][core];
    unsigned long stall_cycles = _end.values[STALL_CYCLES][core]
      - _start.values[STALL_CYCLES][core];
    unsigned long llc_misses = 0;

#if defined(AMD64)
    if (_attr_type[LLC_MISSES] == PER_CLUSTER) {
      int delegate_core = core - core % NUM_CORES_IN_CLUSTER;
      llc_misses =
        (_end.values[LLC_MISSES][delegate_core] -
         _start.values[LLC_MISSES][delegate_core]);
    } else {
      llc_misses =
        (_end.values[LLC_MISSES][core] - _start.values[LLC_MISSES][core]);
    }
#elif defined(AMD32)
    if (_attr_type[LLC_MISSES] == PER_CLUSTER) {
      int delegate_core = core - core % NUM_CORES_IN_CLUSTER;
      llc_misses =
        (_end.values[LLC_MISSES][delegate_core] -
         _start.values[LLC_MISSES][delegate_core])/NUM_CORES_IN_CLUSTER;
    } else {
      llc_misses =
        (_end.values[LLC_MISSES][core] - _start.values[LLC_MISSES][core]);
    }
#elif defined(TILEGX36)
    llc_misses =
      (_end.values[LOCAL_DATA_READ_MISS][core]
       - _start.values[LOCAL_DATA_READ_MISS][core]);
    llc_misses +=
      (_end.values[LOCAL_WRITE_MISS][core]
       - _start.values[LOCAL_WRITE_MISS][core]);
    llc_misses +=
      (_end.values[REMOTE_DATA_READ_MISS][core]
       - _start.values[REMOTE_DATA_READ_MISS][core]);
    llc_misses +=
      (_end.values[REMOTE_WRITE_MISS][core]
       - _start.values[REMOTE_WRITE_MISS][core]);
#endif

    _sum_llc_misses += llc_misses;
    _sum_total_cycles += total_cycles;

    //dout << "jiffies_in_usecs: " << jiffies
    //    << " core: " << core
    //    << " TOT_CYC: " << total_cycles
    //    << " TOTAL_INSTRUCTIONS: " << total_instructions
    //    << " STALL_CYCLES: " << stall_cycles
    //    << " LLC_MISSES: " << llc_misses << endl;

    dout << jiffies
        << "\t" << core
        << "\t" << total_cycles
        << "\t" << total_instructions
        << "\t" << stall_cycles
        << "\t" << llc_misses << endl;

    //std::cout << "core: " << core << " TOT_CYC: " << total_cycles
    //  << " LLC_MISSES: " << llc_misses << std::endl;
  }
}

