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
/// @decription profiler.h
/// 
//------------------------------------------------------------------------------

#include "event.h"
#include "pmu_interface.h"

#include <map>
#include <thread>
#include <cstdio>
#include <iostream>

//------------------------------------------------------------------------------
/// @brief the profiler interface
///
class CProfiler {
  public:
    /// @name constructors/destructors
    /// @{

    /// @brief default constructor
    CProfiler() {}

    /// @brief default destructor
    ~CProfiler() {}

    /// @}

    /// @name profiler attributes
    /// @{

    /// @brief start profiling
    virtual void StartProfiling(void) = 0;

    /// @brief stop profiling
    virtual void StopProfiling(void) = 0;

    /// @}
};

//------------------------------------------------------------------------------
/// @brief simple online profiler
/// 
class CSimpleProfiler: public CProfiler {
  public:
    /// @name constructors/destructors
    /// @{

    /// @brief default constructor
    CSimpleProfiler();

    /// @brief default destructor
    ~CSimpleProfiler();

    /// @}

    /// @name profiler attributes
    /// @{

    /// @brief system-wide profiling (start to read performance counters)
    virtual void StartProfiling(void);

    /// @brief system-wide profiling (stop to read performance counters)
    ///
    /// @description update total cycles and llc misses to compute llc miss rate
    virtual void StopProfiling(void);

    /// @}

  private:
    //
    PMUReqManager*  _pmu_manager;  ///< pmu request manager

    std::pair<__u32, __u64>* _attr_mapping;
    event_type* _attr_type;
    event_values _start;           ///< pmu counter values at StartProfiling()

    unsigned long _sum_total_cycles;
    unsigned long _sum_llc_misses;
};

