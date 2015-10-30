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

struct taskprofile_user_data {
  int counts;
  int initial_state;
	int resume_counts;
	int suspend_counts;
	unsigned long* resume_time;
	unsigned long* suspend_time;
};
struct taskprofile_user_data {
  int needbytes;
  int cpu_counts;
  struct taskprofile_user_cpu_data *cpu_data;
}

#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IORW(MAJOR_NUM, 3, struct taskprofile_user_data*)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101
