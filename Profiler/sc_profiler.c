//------------------------------------------------------------------------------
/// @brief Dynamic RTE profiler implementation
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/01 Younghyun Cho created @n
/// 2015/01 Suwon Oh      modified sched_sched_cpuset
/// 2015/03 Younghyun Cho simple task allocator (using linux load balancer)
/// 2015/04 Younghyun Cho code clean
///
/// @section license_section Licence
/// Copyright (c) 2014,2015, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription sc_profiler.c
/// SC-profiler kernel module implementation.
/// it manages hardware resources for each parallel(OpenMP/OpenCL) applications.
/// it directly manages each worker thread of parallel applications.
/// currently, its main functionality is to set cpu core affinities
/// of each worker thread by taking master thread's pid.
///
//------------------------------------------------------------------------------

/*
 * sc_profiler.c
 */
#include "sc_profiler.h"

static int profiler_open(struct inode *inode, struct file *file)
{
#if ACCESS_ONLY_ONE
  if (profiler_Open)
    return -EBUSY;
  profiler_Open++;
#endif

  try_module_get(THIS_MODULE);
  return SUCCESS;
}

static int profiler_release(struct inode *inode, struct file *file)
{
#if ACCESS_ONLY_ONE
  profiler_opened--;
#endif
  module_put(THIS_MODULE);
  return SUCCESS;
}

static ssize_t profiler_read(struct file *file,
    char __user *buffer,
    size_t length,
    loff_t *offset)
{
  return SUCCESS;
}

static ssize_t profiler_write(struct file *file,
    const char __user *buffer,
    size_t length,
    loff_t *offset)
{
  return SUCCESS;
}

#ifdef UNLOCKED
int profiler_ioctl(struct file *file,
                   unsigned int ioctl_num,
                   unsigned long arg)
#else
int profiler_ioctl(struct inode *inode, /* see include/linux/fs.h */
                   struct file *file,
                   unsigned int ioctl_num,
                   unsigned long arg)
#endif
{
  int ret = 0;

  switch (ioctl_num) {
    case IOCTL_COMMAND_1: // IOCTL_START_PROFILING
    {
      struct task_struct* task = current;
      task->profiled_data.starting_flag = 1;

      break;
    }
    case IOCTL_COMMAND_2: // IOCTL_STOP_PROFILING
    {
      struct task_struct* task = current;
      task->profiled_data.starting_flag = 0;

      break;
    }
    case IOCTL_COMMAND_3: // IOCTL_DUMP_PROFILED_RESULT
    {
      struct task_struct* task = current;
      struct taskprofile_data profiled_data;
      int i = 0, j = 0;

      for (i=0; i<64; i++) {
        profiled_data.start_execution_cnt[i] =
          task->profiled_data.start_execution_cnt[i];
        profiled_data.end_execution_cnt[i] =
          task->profiled_data.end_execution_cnt[i];
      }

      for (i=0; i<64; i++) {
        for (j=0; j<10000; j++) {
          profiled_data.start_executions[i] =
            task->profiled_data.start_executions[i][j];
          profiled_data.end_executions[i] =
            task->profiled_data.end_executions[i][j];
        }
      }

      ret = copy_to_user((void*)arg, &profiled_data, sizeof(struct taskprofile_data));
      break;
    }
    default:
    {
      printk(KERN_ALERT "no support ioctl command %d", ioctl_num);

      break;
    }
  }

  return ret;
}

int module_start(void)
{
  int ret_val;

  /*
   * Register the character mailbox
   */
  ret_val = register_chrdev(MAJOR_NUM, DEVICE_FILE_NAME, &Fops);
  if (ret_val < 0) {
    printk(KERN_ALERT "%s failed with %d\n",
        "Sorry registering the mailbox", ret_val);
    return ret_val;
  }

  printk(KERN_ALERT "Hello SC profiler!\n");

  return 0;
}

void module_end(void)
{
  unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME);
  printk(KERN_ALERT "Bye SC profiler!\n");
}

module_init(module_start);
module_exit(module_end);

MODULE_LICENSE("GPL");

