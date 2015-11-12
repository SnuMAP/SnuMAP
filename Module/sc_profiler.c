//------------------------------------------------------------------------------
/// @brief Application trace profiler implementation
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created @n
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
		int i = 0;

		for (i = 0; i < 64; i++) {
			task->profile_data.resume_time[i] =
				kmalloc(sizeof(unsigned long)*10000, GFP_KERNEL);
			task->profile_data.suspend_time[i] =
				kmalloc(sizeof(unsigned long)*10000, GFP_KERNEL);
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

		for (j = 0; j < 8; j++) {
			printk(KERN_ALERT ">> cpu: %d resume_cnt: %d suspend_cnt: %d\n",
					j, task->profile_data.resume_cnt[j], task->profile_data.suspend_cnt[j]);

			for (k = 0; k < task->profile_data.resume_cnt[j]; k++) {
				printk(KERN_ALERT ">>>> cnt: %d resume_time: %lu suspend_time: %lu\n",
						k, task->profile_data.resume_time[j][k], task->profile_data.suspend_time[j][k]);
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
			dump_profile_result();
			//struct taskprofile_user_data data;
			//printk(KERN_ALERT "dump_profile_result called\n");
			//data = dump_profile_result();

			//ret = copy_to_user((void*)arg, &data, sizeof(struct taskprofile_user_data));

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

