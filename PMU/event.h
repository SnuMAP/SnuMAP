//------------------------------------------------------------------------------
/// @brief PMU event description
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
/// @decription event.h
/// PMU event description.
///
//------------------------------------------------------------------------------

#ifndef __PMU_EVENT_H_
#define __PMU_EVENT_H_

#include <linux/perf_event.h>
#include "common.h"

/// @brief basic event set
///
/// @description event description (event to perf mapping)
#if defined(AMD64)
#define TOTAL_CYCLES_TYPE            PERF_TYPE_HARDWARE
#define TOTAL_CYCLES_CONFIG          PERF_COUNT_HW_CPU_CYCLES
#define TOTAL_INSTRUCTIONS_TYPE      PERF_TYPE_HARDWARE
#define TOTAL_INSTRUCTIONS_CONFIG    PERF_COUNT_HW_INSTRUCTIONS
#define STALL_CYCLES_TYPE            PERF_TYPE_RAW
#define STALL_CYCLES_CONFIG          0xD1
#define LLC_MISSES_TYPE              6
#define LLC_MISSES_CONFIG            0x40053F7E1
#define L2_CACHE_MISSES_TYPE         PERF_TYPE_RAW
#define L2_CACHE_MISSES_CONFIG       0x53177E

#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_CONFIG    0x1005301E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_CONFIG    0x1005302E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG    0x1005304E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_CONFIG    0x1005308E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_CONFIG    0x1005310E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_CONFIG    0x1005320E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_CONFIG    0x1005340E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_CONFIG    0x1005380E0

#define NUM_CORES                    64
#define NUM_CORES_IN_CLUSTER         8
#define MAX_EVENTS                   4

#elif defined(AMD32)
#define TOTAL_CYCLES_TYPE            PERF_TYPE_HARDWARE
#define TOTAL_CYCLES_CONFIG          PERF_COUNT_HW_CPU_CYCLES
#define TOTAL_INSTRUCTIONS_TYPE      PERF_TYPE_HARDWARE
#define TOTAL_INSTRUCTIONS_CONFIG    PERF_COUNT_HW_INSTRUCTIONS
#define STALL_CYCLES_TYPE            PERF_TYPE_RAW
#define STALL_CYCLES_CONFIG          0xD1
#define LLC_MISSES_TYPE              6
#define LLC_MISSES_CONFIG            0x40053F7E1
#define L2_CACHE_MISSES_TYPE         PERF_TYPE_RAW
#define L2_CACHE_MISSES_CONFIG       0x53177E

#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_0_CONFIG    0x1005301E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_1_CONFIG    0x1005302E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_2_CONFIG    0x1005304E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_3_CONFIG    0x1005308E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_4_CONFIG    0x1005310E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_5_CONFIG    0x1005320E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_6_CONFIG    0x1005340E0
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_TYPE      6
#define CPU_TO_DRAM_REQUESTS_TO_TARGET_NODE_7_CONFIG    0x1005380E0

#define NUM_CORES                    32
#define NUM_CORES_IN_CLUSTER         8
#define MAX_EVENTS                   4

#elif defined(TILEGX36)
#define TOTAL_CYCLES_TYPE            PERF_TYPE_HARDWARE
#define TOTAL_CYCLES_CONFIG          PERF_COUNT_HW_CPU_CYCLES
#define TOTAL_INSTRUCTIONS_TYPE      PERF_TYPE_HARDWARE
#define TOTAL_INSTRUCTIONS_CONFIG    PERF_COUNT_HW_INSTRUCTIONS
#define LOCAL_DATA_READ_MISS_TYPE    PERF_TYPE_RAW
#define LOCAL_DATA_READ_MISS_CONFIG  0x91
#define LOCAL_WRITE_MISS_TYPE        PERF_TYPE_RAW
#define LOCAL_WRITE_MISS_CONFIG      0x92
#define REMOTE_DATA_READ_MISS_TYPE   PERF_TYPE_RAW
#define REMOTE_DATA_READ_MISS_CONFIG 0x94
#define REMOTE_WRITE_MISS_TYPE       PERF_TYPE_RAW
#define REMOTE_WRITE_MISS_CONFIG     0x95

#define NUM_CORES                    36
#define NUM_CORES_IN_CLUSTER         36
#define MAX_EVENTS                   5
#endif

/// @brief event type
enum event_type {
  PER_CORE,    // per core event
  PER_CLUSTER, // per cluster event
};

/// @brief performance counter values
///
/// @description store performance counter in this format
typedef struct {
  unsigned long values[MAX_EVENTS][NUM_CORES];
  unsigned long timestamp[MAX_EVENTS][NUM_CORES];
} event_values;

#endif // __PMU_EVENT_H_

