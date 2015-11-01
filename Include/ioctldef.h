//------------------------------------------------------------------------------
/// @brief ioctl definition header
/// @author Heesik Shin <star114@snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/10 Heesik Shin created
///
/// @section license_section Licence
/// Copyright (c) 2015, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription ioctldef.h
/// 
//------------------------------------------------------------------------------

#define IOCTL_START_PROFILING      _IO(MAJOR_NUM, 1)
#define IOCTL_STOP_PROFILING       _IO(MAJOR_NUM, 2)
#define IOCTL_DUMP_PROFILED_RESULT _IO(MAJOR_NUM, 3)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101
