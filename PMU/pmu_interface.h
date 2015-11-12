//------------------------------------------------------------------------------
/// @brief PMU interface protocol
/// @author Eunjin Song <eunjin@csap.snu.ac.kr> and
///         Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/01 Younghyun Cho created
/// 2015/02 Eunjin Song   implement PMUReqManager
/// 2015/07 Younghyun Cho code refactoring
/// 2015/11 Younghyun Cho apply to trace-profiler
///
/// @section license_section Licence
/// Copyright (c) 2014,2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription pmu_interface.h
/// PMU interface protocol description.
/// 
//------------------------------------------------------------------------------

#ifndef __PMU_INTERFACE_H_
#define __PMU_INTERFACE_H_

#include <cstdlib>
#include <unistd.h>
#include <linux/perf_event.h>
#include <sys/epoll.h>
#include <mutex>

#include <tuple>
#include "common.h"
#include "event.h"
#include <iostream>

//------------------------------------------------------------------------------
/// @brief PMU Request Manager
///
class PMUReqManager {
  public:
    /// @name constructors/destructors
    /// @{

    /// @brief default constructor
    PMUReqManager();

    /// @brief default destructor
    ~PMUReqManager();

    /// @}

    /// @name PMU request manager attributes
    /// @{

    /// @brief perf event attributes initial setting
    void InitPerfEventAttr();

    /// @brief perf event attributes setting from input
    ///
    /// @param attribute mapping information
    /// @param attribute type information
    /// @param the number of registers to be registered
    void InitPerfEventAttr(
        std::pair<__u32, __u64>* attr_mapping,
        event_type* attr_type,
        int num_events);

    /// @brief get current count values
    ///
    /// @param pointer to save current count values
    void GetCurrentCount(event_values& val);

    /// @brief open event file descriptors
    void OpenEventFDs();

    /// @brief open a subset of event file descriptors
    ///
    /// @param starting event index
    /// @description this open event file descriptors from the index to last
    void OpenEventFDs(int subset_index);

    /// @brief close event file descriptors
    void CloseEventFDs();

    /// @brief close a subset of event file descriptors
    ///
    /// @param starting event index
    /// @description this close event file descriptors from the index to last
    void CloseEventFDs(int subset_index);

    /// @brief record count values int ocount values
    void RecordAllCounters();

    /// @}

  private:
    /// @name PMU request manager private attributes
    /// @{

    /// @brief read count values from file descriptors
    void ReadCounters(event_values *values);

    /// @}

    int _fd[MAX_EVENTS][NUM_CORES]; ///< event-perf file descriptors
    struct perf_event_attr _attr[MAX_EVENTS]; ///< event-perf attributes
    event_type _event_type[MAX_EVENTS]; ///< event type information
    event_values _cur; ///< recently stored count values
    std::mutex _cur_mutex;
};

#endif

