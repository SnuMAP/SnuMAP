//------------------------------------------------------------------------------
/// @brief Application trace profiler implementation
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created @n
/// 2015/10 Heesik Shin updated data structure @n
/// 2015/10 Heesik Shin apply IOCTL number macro. @n
///
/// @section license_section Licence
/// Copyright (c) 2015, Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription sc_profiler.c
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

void start_profiling(void)
{
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;

	do {
    // allocate memory
    if (task->profile_data.cpu_data == NULL)
    {
      int i = 0;
      int cpu_counts = num_online_cpus();
      //printk(KERN_ALERT "start_profiling : cpu count: %d\n", cpu_counts); // Test code. 
      task->profile_data.cpu_data
        = kzalloc(sizeof(struct taskprofile_cpu_data) * cpu_counts, GFP_KERNEL);
      // initialize memory.
      for (i = 0; i < cpu_counts; i++)
      {
        task->profile_data.cpu_data[i].initial_state = 0;
        task->profile_data.cpu_data[i].head
          = kzalloc (sizeof(struct taskprofile_list), GFP_KERNEL);
        task->profile_data.cpu_data[i].list_counts = 1;

        // time data allocation and initialization
        task->profile_data.cpu_data[i].head->next = NULL;
        task->profile_data.cpu_data[i].head->resume_counts = 0;
        task->profile_data.cpu_data[i].head->suspend_counts = 0;
        
        task->profile_data.cpu_data[i].head->resume_time
          = kzalloc (sizeof(unsigned long) * MAX_TIME_COUNT, GFP_KERNEL);
        task->profile_data.cpu_data[i].head->suspend_time
          = kzalloc (sizeof(unsigned long) * MAX_TIME_COUNT, GFP_KERNEL);
      }
    }

		task->profile_data.starting_flag = 1;

		task = next_thread(task);
	} while (task != master_thread);
}

void stop_profiling(void)
{
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;

	do {
		task->profile_data.starting_flag = 0;
		task = next_thread(task);
	} while (task != master_thread);
}

int print_taskprofile_list(struct taskprofile_list *tp_current)
{
  int i = 0;
  int list_number = 0;
  int base_number = 0;
  if (tp_current == NULL) return 0;
  else list_number = print_taskprofile_list(tp_current->next);

  base_number = list_number * MAX_TIME_COUNT;
  for (i = 0; i < tp_current->resume_counts; i++) {
    printk(KERN_ALERT ">>>>>> cnt: %d resume_time: %lu suspend_time: %lu\n",
        base_number + i, 
        tp_current->resume_time[i], 
        tp_current->suspend_time[i]);
  }
  return ++list_number;
}

void dump_profile_result(void)
{
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;
	int i = 0;

	do {
		int j = 0;

		printk(KERN_ALERT "thread: %d\n", i);
    
    for (j = 0; j < num_online_cpus(); j++)
    {
      int base_number = MAX_TIME_COUNT * (task->profile_data.cpu_data[j].list_counts-1);
      printk(KERN_ALERT ">> cpu: %d initial_state: %d list_counts: %d\n", 
          j, 
          task->profile_data.cpu_data[j].initial_state,
          task->profile_data.cpu_data[j].list_counts);
      printk(KERN_ALERT ">>>> resume_cnt: %d suspend_cnt: %d\n",
            base_number + task->profile_data.cpu_data[j].head->resume_counts, 
            base_number + task->profile_data.cpu_data[j].head->suspend_counts); 
      print_taskprofile_list(task->profile_data.cpu_data[j].head);
    }

		task = next_thread(task);
		i++;

	} while (task != master_thread);

	return;
}

#ifdef UNLOCKED
long profiler_ioctl(struct file *file,
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

	switch (_IOC_NR(ioctl_num)) {
    case _IOC_NR(IOCTL_START_PROFILING):
		{
			printk(KERN_ALERT "start_profiling called\n");
			start_profiling();

			break;
		}
    case _IOC_NR(IOCTL_STOP_PROFILING):
		{
			printk(KERN_ALERT "stop_profiling called\n");
			stop_profiling();

			break;
		}
    case _IOC_NR(IOCTL_DUMP_PROFILED_RESULT):
		{
			printk(KERN_ALERT "dump_profiled_result called\n");
			dump_profile_result();

			break;
		}
		default:
		{
			printk(KERN_ALERT "no support ioctl command %d\n", ioctl_num);

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

	printk(KERN_ALERT "Hello OpenMP trace profiler!\n");

	return 0;
}

void module_end(void)
{
	unregister_chrdev(MAJOR_NUM, DEVICE_FILE_NAME);
	printk(KERN_ALERT "Bye OpenMP trace profiler!\n");
}

module_init(module_start);
module_exit(module_end);

MODULE_LICENSE("GPL");

