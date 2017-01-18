//------------------------------------------------------------------------------
/// NUMA performance profiler
/// 

#include "pmu_interface.h"

#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <fstream>

#include <pthread.h>

class CNUMAPerformance {
  public:
    CNUMAPerformance(int num_nodes, int num_cores);
    ~CNUMAPerformance();

    void AddValue(unsigned long* l2_requests,
        unsigned long* l2_misses,
        unsigned long* l3_requests,
        unsigned long* l3_misses,
        unsigned long** dram_reqs);

    unsigned long* l2_requests_;
    unsigned long* l2_misses_;
    unsigned long* l3_requests_;
    unsigned long* l3_misses_;
    unsigned long** dram_reqs_;

    int num_nodes_;
    int num_cores_;
};

class CProfiler {
  public:
    virtual void StartProfiling() = 0;
    virtual bool StopProfiling(unsigned long* l2_accesses,
        unsigned long* l2_misses,
        unsigned long* l3_accesses,
        unsigned long* l3_misses,
        unsigned long* local_accesses,
        unsigned long* remote_accesses) = 0;
    virtual void InitPerfEventAttr() = 0;
    virtual void DumpProfilingResults() = 0;
};

class CNUMAProfiler: public CProfiler {
  public:
    CNUMAProfiler(int num_nodes, int num_cores, int cores_in_node);
    ~CNUMAProfiler();

    virtual void StartProfiling();
    virtual bool StopProfiling(unsigned long* l2_accesses,
        unsigned long* l2_misses,
        unsigned long* l3_accesses,
        unsigned long* l3_misses,
        unsigned long* local_accesses,
        unsigned long* remote_accesses);
    bool StopProfilingExt(unsigned long* l2_requests,
        unsigned long* l2_misses,
        unsigned long* l3_requests,
        unsigned long* l3_misses,
        unsigned long** dram_reqs);
    virtual void InitPerfEventAttr();
    virtual void DumpProfilingResults();

  private:
    void DoAttributeToggling();

    PMUReqManager* pmu_manager_;

    CNUMAPerformance* performance_data_;

    std::pair<__u32, __u64>* attr_mapping_;
    event_type* attr_type_;
    sc_event_values start_;

    unsigned long* l2_requests_;
    unsigned long* l2_misses_;
    unsigned long* l3_requests_;
    unsigned long* l3_misses_;
    unsigned long** dram_reqs_;

    const int L2_REQUESTS = 0;
    const int L2_MISSES = 1;
    const int L3_REQUESTS = 2;
    const int L3_MISSES = 3;
    const int DRAM_REQS_1 = 4;
    const int DRAM_REQS_2 = 5;

    const int num_nodes_;
    const int num_cores_;
    const int cores_in_node_;

    const int num_events_ = 6;
    int toggle_;
};

class CSimpleNUMAProfiler: public CProfiler {
  public:
    CSimpleNUMAProfiler(int num_nodes, int num_cores, int cores_in_node);
    ~CSimpleNUMAProfiler();

    virtual void StartProfiling();
    virtual bool StopProfiling(unsigned long* l2_accesses,
        unsigned long* l2_misses,
        unsigned long* l3_accesses,
        unsigned long* l3_misses,
        unsigned long* local_accesses,
        unsigned long* remote_accesses);
    virtual void InitPerfEventAttr();
    virtual void DumpProfilingResults();

  private:
    PMUReqManager* pmu_manager_;

    std::pair<__u32, __u64>* attr_mapping_;
    event_type* attr_type_;
    sc_event_values start_;

    const int num_nodes_;
    const int num_cores_;
    const int cores_in_node_;

    const int L2_ACCESSES = 0;
    const int L2_MISSES = 1;
    const int L3_ACCESSES = 2;
    const int L3_MISSES = 3;
    const int LOCAL_ACCESSES = 4;
    const int REMOTE_ACCESSES = 5;

    const int num_events_ = 6;
};

