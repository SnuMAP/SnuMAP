//------------------------------------------------------------------------------
/// @brief Dynamic RTE application and RTE communication interface define
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/03 Younghyun Cho created
/// 2015/04 Younghyun Cho dynamic RTE
/// 2016/03 Younghyun Cho simlify the interface to support only
///                       asynchronous communication model
///
/// @section license_section Licence
/// Copyright (c) 2015, 2016, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription comm_interface.h
/// define communication headers between RTE and application runtimes
/// 
//------------------------------------------------------------------------------

#ifndef __LIB_INTERFACE_H_
#define __LIB_INTERFACE_H_

#define APP_BEGIN         1
#define APP_END           4

typedef struct {
  int pid;          ///< application thread id (master thread)
  char name[4096];  ///< application name
} __attribute__((packed))app_begin_data;

typedef struct {
  int pid;
} __attribute__((packed))app_end_data;

typedef struct {
  int region_id;
  int delegate_id;
  int available[64];
  char kernel_name[4096];
} __attribute__((packed))app_context;

#endif // __LIB_INTERFACE_H_
