//------------------------------------------------------------------------------
/// NUMA performance profiler
/// 

#include "profiler.h"
#include <unistd.h>

//#define READ_COMMAND_ONLY

CNUMAPerformance::CNUMAPerformance(int num_nodes, int num_cores)
{
  num_nodes_ = num_nodes;
  num_cores_ = num_cores;

  l2_requests_ = (unsigned long*)malloc(num_cores_ * sizeof(unsigned long));
  l2_misses_ = (unsigned long*)malloc(num_cores_ * sizeof(unsigned long));
  l3_requests_ = (unsigned long*)malloc(num_nodes_ * sizeof(unsigned long));
  l3_misses_ = (unsigned long*)malloc(num_nodes_ * sizeof(unsigned long));
  dram_reqs_ = (unsigned long**)malloc(num_nodes_ * sizeof(unsigned long*));

  for (int node = 0; node < num_nodes_; node++)
    dram_reqs_[node] = (unsigned long*)malloc(num_nodes_ * sizeof(unsigned long));

  for (int core = 0; core < num_cores_; core++) {
    l2_requests_[core] = 0;
    l2_misses_[core] = 0;
  }

  for (int node = 0; node < num_nodes_; node++) {
    l3_requests_[node] = 0;
    l3_misses_[node] = 0;
    for (int dram = 0; dram < num_nodes_; dram++)
      dram_reqs_[node][dram] = 0;
  }
}

CNUMAPerformance::~CNUMAPerformance()
{
  free(l2_requests_);
  free(l2_misses_);
  free(l3_requests_);
  free(l3_misses_);
  for (int node = 0; node < num_nodes_; node++)
    free(dram_reqs_[node]);
  free(dram_reqs_);
}

void CNUMAPerformance::AddValue(unsigned long* l2_requests,
    unsigned long* l2_misses,
    unsigned long* l3_requests,
    unsigned long* l3_misses,
    unsigned long** dram_reqs)
{
  for (int core = 0; core < num_cores_; core++) {
    l2_requests_[core] += l2_requests[core];
    l2_misses_[core] += l2_misses[core];
  }

  for (int node = 0; node < num_nodes_; node++) {
    l3_requests_[node] += l3_requests[node];
    l3_misses_[node] += l3_misses[node];
    for (int dram = 0; dram < num_nodes_; dram++) {
      dram_reqs_[node][dram] += dram_reqs[node][dram];
    }
  }
}

CNUMAProfiler::CNUMAProfiler(int num_nodes, int num_cores, int cores_in_node)
  : num_nodes_(num_nodes), num_cores_(num_cores), cores_in_node_(cores_in_node)
{
#if defined(AMD64)
  pmu_manager_ = new PMUReqManager(num_cores_, cores_in_node_, num_events_);

  performance_data_ = new CNUMAPerformance(num_nodes_, num_cores_);

  attr_mapping_ = (std::pair<__u32, __u64>*)
    malloc(num_events_ * sizeof(std::pair<__u32, __u64>));
  attr_type_ = (event_type*)malloc(num_events_ * sizeof(event_type));

  l2_requests_ = (unsigned long*)malloc(num_cores_ * sizeof(unsigned long));
  l2_misses_ = (unsigned long*)malloc(num_cores_ * sizeof(unsigned long));
  l3_requests_ = (unsigned long*)malloc(num_nodes_ * sizeof(unsigned long));
  l3_misses_ = (unsigned long*)malloc(num_nodes_ * sizeof(unsigned long));
  dram_reqs_ = (unsigned long**)malloc(num_nodes_ * sizeof(unsigned long*));

  for (int node = 0; node < num_nodes_; node++)
    dram_reqs_[node] = (unsigned long*)malloc(num_nodes_ * sizeof(unsigned long));

  for (int core = 0; core < num_cores_; core++) {
    l2_requests_[core] = 0;
    l2_misses_[core] = 0;
  }

  for (int node = 0; node < num_nodes_; node++) {
    l3_requests_[node] = 0;
    l3_misses_[node] = 0;
    for (int dram = 0; dram < num_nodes_; dram++)
      dram_reqs_[node][dram] = 0;
  }
#endif
}

CNUMAProfiler::~CNUMAProfiler()
{
#if defined(AMD64)
  free(attr_mapping_);
  free(attr_type_);

  free(l2_requests_);
  free(l2_misses_);
  free(l3_requests_);
  free(l3_misses_);
  for (int node = 0; node < num_nodes_; node++)
    free(dram_reqs_[node]);
  free(dram_reqs_);

  delete pmu_manager_;
  delete performance_data_;
#endif
}

void CNUMAProfiler::StartProfiling()
{
#if defined(AMD64)
  pmu_manager_->RecordAllCounters();
  pmu_manager_->GetCurrentCount(start_);
#endif
}

bool CNUMAProfiler::StopProfiling(unsigned long* l2_accesses,
    unsigned long* l2_misses,
    unsigned long* l3_accesses,
    unsigned long* l3_misses,
    unsigned long* local_accesses,
    unsigned long* remote_accesses)
{
  return false;
}

bool CNUMAProfiler::StopProfilingExt(unsigned long* l2_requests,
    unsigned long* l2_misses,
    unsigned long* l3_requests,
    unsigned long* l3_misses,
    unsigned long** dram_reqs)
{
#if defined(AMD64)
  sc_event_values end_;
  pmu_manager_->RecordAllCounters();
  pmu_manager_->GetCurrentCount(end_);

  if (end_.values[0][0] == start_.values[0][0]) {
    std::cout << "end value and start value are same" << std::endl;
    DoAttributeToggling();
    return false;
  }

  for (int core = 0; core < num_cores_; core++) {
    l2_requests_[core] +=
      end_.values[L2_REQUESTS][core] - start_.values[L2_REQUESTS][core];
    l2_misses_[core] +=
      end_.values[L2_MISSES][core] - start_.values[L2_MISSES][core];
  }

  for (int node = 0; node < num_nodes_; node++) {
    int cpu = node * cores_in_node_;
    l3_requests_[node] +=
      end_.values[L3_REQUESTS][cpu] - start_.values[L3_REQUESTS][cpu];
    l3_misses_[node] +=
      end_.values[L3_MISSES][cpu] - start_.values[L3_MISSES][cpu];
    dram_reqs_[node][toggle_*2] +=
      end_.values[DRAM_REQS_1][cpu] - start_.values[DRAM_REQS_1][cpu];
    dram_reqs_[node][toggle_*2+1] +=
      end_.values[DRAM_REQS_2][cpu] - start_.values[DRAM_REQS_2][cpu];
  }

  if (toggle_ == 3) {
    for (int core = 0; core < num_cores_; core++) {
      l2_requests[core] += l2_requests_[core];
      l2_misses[core] += l2_misses_[core];
    }

    for (int node = 0; node < num_nodes_; node++) {
      l3_requests[node] += l3_requests_[node];
      l3_misses[node] += l3_misses_[node];
      for (int dram = 0; dram < num_nodes_; dram++) {
        dram_reqs[node][dram] += dram_reqs_[node][dram];
      }
    }

    for (int core = 0; core < num_cores_; core++) {
      l2_requests_[core] = 0;
      l2_misses_[core] = 0;
    }

    for (int node = 0; node < num_nodes_; node++) {
      l3_requests_[node] = 0;
      l3_misses_[node] = 0;
      for (int dram = 0; dram < num_nodes_; dram++)
        dram_reqs_[node][dram] = 0;
    }

    DoAttributeToggling();
    return true;
  } else {
    DoAttributeToggling();
    return false;
  }

#endif
  return true;
}

void CNUMAProfiler::InitPerfEventAttr()
{
#if defined(AMD64)
  pmu_manager_->CloseEventFDs();

  attr_mapping_[L2_REQUESTS] = std::make_pair<__u32, __u64>
    (REQUESTS_TO_L2_TYPE, REQUESTS_TO_L2_CONFIG);
  attr_mapping_[L2_MISSES] = std::make_pair<__u32, __u64>
    (L2_CACHE_MISS_TYPE, L2_CACHE_MISS_CONFIG);
  attr_mapping_[L3_REQUESTS] = std::make_pair<__u32, __u64>
    (READ_REQUEST_TO_L3_CACHE_TYPE, READ_REQUEST_TO_L3_CACHE_CONFIG);
  attr_mapping_[L3_MISSES] = std::make_pair<__u32, __u64>
    (L3_CACHE_MISSES_TYPE, L3_CACHE_MISSES_CONFIG);

#ifdef READ_COMMAND_ONLY
  attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
    (CPU_READ_COMMAND_REQUESTS_TO_NODE_0_TYPE,
     CPU_READ_COMMAND_REQUESTS_TO_NODE_0_CONFIG);
  attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
    (CPU_READ_COMMAND_REQUESTS_TO_NODE_1_TYPE,
     CPU_READ_COMMAND_REQUESTS_TO_NODE_1_CONFIG);
#else
  attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
    (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_TYPE,
     CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_CONFIG);
  attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
    (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_TYPE,
     CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_CONFIG);
#endif

  attr_type_[L2_REQUESTS] = PER_CORE;
  attr_type_[L2_MISSES] = PER_CORE;
  attr_type_[L3_REQUESTS] = PER_NODE;
  attr_type_[L3_MISSES] = PER_NODE;
  attr_type_[DRAM_REQS_1] = PER_NODE;
  attr_type_[DRAM_REQS_2] = PER_NODE;

  pmu_manager_->InitPerfEventAttr(attr_mapping_, attr_type_);
  toggle_ = 0;
  for (int core = 0; core < num_cores_; core++) {
    l2_requests_[core] = 0;
    l2_misses_[core] = 0;
  }
  for (int node = 0; node < num_nodes_; node++) {
    l3_requests_[node] = 0;
    l3_misses_[node] = 0;
    for (int dram = 0; dram < num_nodes_; dram++)
      dram_reqs_[node][dram] = 0;
  }

  pmu_manager_->OpenEventFDs();
#endif
}

void CNUMAProfiler::DoAttributeToggling()
{
#if defined(AMD64)
  pmu_manager_->CloseEventFDs(4); // DRAM_REQS_1 and DRAM_REQS_2

#ifdef READ_COMMAND_ONLY
  if (toggle_ == 0) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_2_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_2_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_3_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_3_CONFIG);
  } else if (toggle_ == 1) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_4_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_4_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_5_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_5_CONFIG);
  } else if (toggle_ == 2) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_6_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_6_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_7_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_7_CONFIG);
  } else if (toggle_ == 3) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_0_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_0_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_READ_COMMAND_REQUESTS_TO_NODE_1_TYPE,
       CPU_READ_COMMAND_REQUESTS_TO_NODE_1_CONFIG);
  }
#else
  if (toggle_ == 0) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_CONFIG);
  } else if (toggle_ == 1) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_CONFIG);
  } else if (toggle_ == 2) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_CONFIG);
  } else if (toggle_ == 3) {
    attr_mapping_[DRAM_REQS_1] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_CONFIG);
    attr_mapping_[DRAM_REQS_2] = std::make_pair<__u32, __u64>
      (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_TYPE,
       CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_CONFIG);
  }
#endif

  toggle_ = (toggle_ + 1) % 4;

  pmu_manager_->InitPerfEventAttr(attr_mapping_, attr_type_);
  pmu_manager_->OpenEventFDs(4); // DRAM_REQS_1 and DRAM_REQS_2
#endif
}

void CNUMAProfiler::DumpProfilingResults()
{
#if defined(AMD64)
  for (int node = 0; node < num_nodes_; node++) {
    //unsigned long sum_node_l2_requests = 0;
    //unsigned long sum_node_l2_misses = 0;
    //for (int core = 0; core < cores_in_node_; core++) {
    //  unsigned long sum_l2_requests =
    //    performance_data_->l2_requests_[node*cores_in_node_+core];
    //  unsigned long sum_l2_misses =
    //    performance_data_->l2_misses_[node*cores_in_node_+core];
    //  fprintf(stdout, "node_%d_core_%d_sum_l2_requests: %lu\n",
    //      node, core, sum_l2_requests);
    //  fprintf(stdout, "node_%d_core_%d_sum_l2_misses: %lu\n",
    //      node, core, sum_l2_misses);
    //  sum_node_l2_requests += sum_l2_requests;
    //  sum_node_l2_misses += sum_l2_misses;
    //}

    //fprintf(stdout, "node_%d_sum_l2_requests: %lu\n",
    //    node, sum_node_l2_requests);
    //fprintf(stdout, "node_%d_sum_l2_misses: %lu\n",
    //    node, sum_node_l2_misses);

    //unsigned long sum_l3_requests = performance_data_->l3_requests_[node];
    //fprintf(stdout, "node_%d_sum_l3_requests: %lu\n",
    //    node, sum_l3_requests);

    //unsigned long sum_l3_misses = performance_data_->l3_misses_[node];
    //fprintf(stdout, "node_%d_sum_l3_misses: %lu\n",
    //    node, sum_l3_misses);

    for (int dram = 0; dram < num_nodes_; dram++) {
      unsigned long sum_dram_reqs = performance_data_->dram_reqs_[node][dram];
      fprintf(stdout, "node_%d_dram_%d_requests, %lu\n",
          node, dram, sum_dram_reqs);
    }

    fprintf(stdout, "\n");
  }
#endif
}

CSimpleNUMAProfiler::CSimpleNUMAProfiler(int num_nodes, int num_cores, int cores_in_node)
  : num_nodes_(num_nodes), num_cores_(num_cores), cores_in_node_(cores_in_node)
{
  pmu_manager_ = new PMUReqManager(num_cores_, cores_in_node_, num_events_);

  attr_mapping_ = (std::pair<__u32, __u64>*)
    malloc(num_events_ * sizeof(std::pair<__u32, __u64>));
  attr_type_ = (event_type*)malloc(num_events_ * sizeof(event_type));
}

CSimpleNUMAProfiler::~CSimpleNUMAProfiler()
{
  free(attr_mapping_);
  free(attr_type_);

  delete pmu_manager_;
}

void CSimpleNUMAProfiler::StartProfiling()
{
  pmu_manager_->RecordAllCounters();
  pmu_manager_->GetCurrentCount(start_);
}

bool CSimpleNUMAProfiler::StopProfiling(unsigned long* l2_accesses,
    unsigned long* l2_misses,
    unsigned long* l3_accesses,
    unsigned long* l3_misses,
    unsigned long* local_accesses,
    unsigned long* remote_accesses)
{
  sc_event_values end_;
  pmu_manager_->RecordAllCounters();
  pmu_manager_->GetCurrentCount(end_);

  if (end_.values[0][0] == start_.values[0][0]) {
    std::cout << "end value and start value are same" << std::endl;
    return false;
  }

  for (int core = 0; core < num_cores_; core++) {
    l2_accesses[core] +=
      end_.values[L2_ACCESSES][core] - start_.values[L2_ACCESSES][core];
    l2_misses[core] +=
      end_.values[L2_MISSES][core] - start_.values[L2_MISSES][core];
  }

  for (int node = 0; node < num_nodes_; node++) {
    int cpu = node * cores_in_node_;
    l3_accesses[node] +=
      end_.values[L3_ACCESSES][cpu] - start_.values[L3_ACCESSES][cpu];
    l3_misses[node] +=
      end_.values[L3_MISSES][cpu] - start_.values[L3_MISSES][cpu];
    local_accesses[node] +=
      end_.values[LOCAL_ACCESSES][cpu] - start_.values[LOCAL_ACCESSES][cpu];
    remote_accesses[node] +=
      (end_.values[REMOTE_ACCESSES][cpu] - start_.values[REMOTE_ACCESSES][cpu]) -
      local_accesses[node];
  }

  return true;
}

void CSimpleNUMAProfiler::InitPerfEventAttr()
{
  pmu_manager_->CloseEventFDs();

  attr_mapping_[L2_ACCESSES] = std::make_pair<__u32, __u64>
    (REQUESTS_TO_L2_TYPE, REQUESTS_TO_L2_CONFIG);
  attr_mapping_[L2_MISSES] = std::make_pair<__u32, __u64>
    (L2_CACHE_MISS_TYPE, L2_CACHE_MISS_CONFIG);
  attr_mapping_[L3_ACCESSES] = std::make_pair<__u32, __u64>
    (READ_REQUEST_TO_L3_CACHE_TYPE, READ_REQUEST_TO_L3_CACHE_CONFIG);
  attr_mapping_[L3_MISSES] = std::make_pair<__u32, __u64>
    (L3_CACHE_MISSES_TYPE, L3_CACHE_MISSES_CONFIG);

  attr_mapping_[REMOTE_ACCESSES] = std::make_pair<__u32, __u64>
    (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_ALL_TYPE,
     CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_ALL_CONFIG);

  attr_type_[L2_ACCESSES]     = PER_CORE;
  attr_type_[L2_MISSES]       = PER_CORE;
  attr_type_[L3_ACCESSES]     = PER_NODE;
  attr_type_[L3_MISSES]       = PER_NODE;
  attr_type_[LOCAL_ACCESSES]  = PER_NODE;
  attr_type_[REMOTE_ACCESSES] = PER_NODE;

  for (int node = 0; node < num_nodes_; node++) {
    __u32 local_access_type;
    __u64 local_access_config;

    if (node == 0) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_CONFIG;
    } else if (node == 1) {
#ifdef AMD32
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG;
#else
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_CONFIG;
#endif
    } else if (node == 2) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG;
    } else if (node == 3) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_CONFIG;
    } else if (node == 4) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_CONFIG;
    } else if (node == 5) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_CONFIG;
    } else if (node == 6) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_CONFIG;
    } else if (node == 7) {
      local_access_type   = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_TYPE;
      local_access_config = CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_CONFIG;
    }

    for (int core = node * cores_in_node_;
        core < (node + 1) * cores_in_node_;
        core++) {
      //attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
      //  (local_access_type, local_access_config);
      if (node == 0)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_CONFIG);
      else if (node == 1)
#ifdef AMD32
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG);
#else
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_CONFIG);
#endif
      else if (node == 2)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG);
      else if (node == 3)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_CONFIG);
      else if (node == 4)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_CONFIG);
      else if (node == 5)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_CONFIG);
      else if (node == 6)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_CONFIG);
      else if (node == 7)
          attr_mapping_[LOCAL_ACCESSES] = std::make_pair<__u32, __u64>
              (CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_TYPE,
               CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_CONFIG);

      pmu_manager_->InitPerfEventAttr(core, attr_mapping_, attr_type_);
    }
  }

  pmu_manager_->OpenEventFDs();
}

void CSimpleNUMAProfiler::DumpProfilingResults()
{
  return;
}

