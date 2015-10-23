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
        = kmalloc(sizeof(struct taskprofile_cpu_data) * cpu_counts, GFP_KERNEL);
      // initialize memory.
      for (i = 0; i < cpu_counts; i++)
      {
        task->profile_data.cpu_data[i].initial_state = -1;
        task->profile_data.cpu_data[i].head
          = kmalloc (sizeof(struct taskprofile_list), GFP_KERNEL);
        task->profile_data.cpu_data[i].list_counts = 1;

        // time data allocation and initialization
        task->profile_data.cpu_data[i].head->next = NULL;
        task->profile_data.cpu_data[i].head->resume_counts = 0;
        task->profile_data.cpu_data[i].head->suspend_counts = 0;
        
        task->profile_data.cpu_data[i].head->resume_time
          = kmalloc (sizeof(unsigned long) * MAX_TIME_COUNT, GFP_KERNEL);
        task->profile_data.cpu_data[i].head->suspend_time
          = kmalloc (sizeof(unsigned long) * MAX_TIME_COUNT, GFP_KERNEL);
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

void dump_profile_result(void)
{
	//struct taskprofile_user_data data;
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;
	int i = 0;

	do {
		int j = 0, k = 0;

		printk(KERN_ALERT "thread: %d\n", i);
    
    for (j = 0; j < num_online_cpus(); j++)
    {
      printk(KERN_ALERT ">> cpu: %d state: %d resume_cnt: %d suspend_cnt: %d\n",
					j,
          task->profile_data.cpu_data[j].initial_state,
          task->profile_data.cpu_data[j].head->resume_counts, 
          task->profile_data.cpu_data[j].head->suspend_counts);

			for (k = 0; k < task->profile_data.cpu_data[j].head->resume_counts; k++) {
				printk(KERN_ALERT ">>>> cnt: %d resume_time: %lu suspend_time: %lu\n",
						k, 
            task->profile_data.cpu_data[j].head->resume_time[k], 
            task->profile_data.cpu_data[j].head->suspend_time[k]);
			}

    }

//		if (i == 0) {
//			for (j = 0; j < 64; j++) {
//				data.resume_cnt[j]
//					= task->profile_data.resume_cnt[j];
//				data.suspend_cnt[j]
//					= task->profile_data.suspend_cnt[j];
//
//				for (k = 0; k < 10000; k++) {
//					data.resume_time[j][k]
//						= task->profile_data.resume_time[j][k];
//					data.suspend_time[j][k]
//						= task->profile_data.suspend_time[j][k];
//				}
//			}
//		}
//
//		for (j = 0; j < 8; j++) {
//			printk(KERN_ALERT "task: %d cpu: %d resume: %d suspend: %d\n",
//					task->pid, j, task->profile_data.resume_cnt[j], task->profile_data.suspend_cnt[j]);
//		}

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

	switch (ioctl_num) {
		//case IOCTL_COMMAND_1: // IOCTL_START_PROFILING
		case 1:
		{
			printk(KERN_ALERT "start_profiling called\n");
			start_profiling();

			break;
		}
		//case IOCTL_COMMAND_2: // IOCTL_STOP_PROFILING
		case 2:
		{
			printk(KERN_ALERT "stop_profiling called\n");
			stop_profiling();

			break;
		}
		//case IOCTL_COMMAND_3: // IOCTL_DUMP_PROFILED_RESULT
		case 3:
		{
			printk(KERN_ALERT "dump_profiled_result called\n");
			dump_profile_result();
			//struct taskprofile_user_data data;
			//printk(KERN_ALERT "dump_profile_result called\n");
			//data = dump_profile_result();

			//ret = copy_to_user((void*)arg, &data, sizeof(struct taskprofile_user_data));

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

