typedef struct {
  int start_execution_cnt[64];
  int end_execution_cnt[64];
  unsigned long start_executions[64][10000];
  unsigned long end_executions[64][10000];
} __attribute__((packed))taskprofile_data;

#define MAJOR_NUM 101
#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, taskprofile_data)

#define IOCTL_COMMAND_1 IOCTL_START_PROFILING
#define IOCTL_COMMAND_2 IOCTL_STOP_PROFILING
#define IOCTL_COMMAND_3 IOCTL_DUMP_PROFILED_RESULT

#define DEVICE_FILE_NAME "profiler_mailbox"
