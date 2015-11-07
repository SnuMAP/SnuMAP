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

// file operation
struct file* file_open(const char* path, int flags, int rights)
{
  struct file* filp = NULL;
  mm_segment_t oldfs;
  int err = 0;

  oldfs = get_fs();
  set_fs(get_ds());
  filp = filp_open(path, flags, rights);
  set_fs(oldfs);
  if(IS_ERR(filp)) {
    err = PTR_ERR(filp);
    printk(KERN_ALERT "file_open returns NULL with flip error\n");
    return NULL;
  }
  return filp;
}
void file_close(struct file* file)
{
  filp_close(file, NULL);
}
int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
  mm_segment_t oldfs;
  int ret;

  oldfs = get_fs();
  set_fs(get_ds());

  ret = vfs_read(file, data, size, &offset);

  set_fs(oldfs);
  return ret;
}
int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size)
{
  mm_segment_t oldfs;
  int ret;

  oldfs = get_fs();
  set_fs(get_ds());

  ret = vfs_write(file, data, size, &offset);

  set_fs(oldfs);
  return ret;
}
int file_sync(struct file* file)
{
  vfs_fsync(file, 0);
  return 0;
}
void print_log(struct file_write_data* fw_data, const char *fmt, ...)
{
  va_list args;
  int i;
  char* buf = kzalloc(256, GFP_ATOMIC);
  
  if (buf)
  {
    va_start(args, fmt);
    i = vsnprintf(buf, 256, fmt, args);
    if (fw_data == NULL || fw_data->file == NULL)
    {
      printk(KERN_ALERT "%s", buf);
    }
    else
    {
      // write to file log.
      // have to consider what offset which file ends is. 
      fw_data->offset += file_write(fw_data->file, fw_data->offset, buf, i);
    }
    va_end(args);
    kfree(buf);
  }
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

int print_taskprofile_list(struct file_write_data* fw_data, struct taskprofile_list *tp_current)
{
  int i = 0;
  int list_number = 0;
  int base_number = 0;
  if (tp_current == NULL) return 0;
  else list_number = print_taskprofile_list(fw_data, tp_current->next);

  base_number = list_number * MAX_TIME_COUNT;
  for (i = 0; i < tp_current->resume_counts; i++) {
    print_log(fw_data, ">>>>>> cnt: %d resume_time: %lu suspend_time: %lu\n",
        base_number + i, 
        tp_current->resume_time[i], 
        tp_current->suspend_time[i]);
  }
  return ++list_number;
}

char * strcat(char *dest, const char *src)
{
  int i,j;
  for (i = 0; dest[i] != '\0'; i++)
    ;
  for (j = 0; src[j] != '\0'; j++)
    dest[i+j] = src[j];
  dest[i+j] = '\0';
  return dest;
}

char * strcpy(char *dest, const char *src)
{
  int i;
  for (i=0; src[i] != '\0'; i++)
    dest[i] = src[i];
  dest[i] = '\0';
  return dest;
}

char* get_exe_path(struct mm_struct *mm)
{
  char *p = NULL;
  if (mm) {
    down_read(&mm->mmap_sem);
    if (mm->exe_file) {
      char *pathname = kzalloc(PATH_MAX, GFP_ATOMIC);
      if (pathname) {
        p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);
      }
    }
    up_read(&mm->mmap_sem);
  }
  return p;
}

void dump_profile_result(void)
{
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;
	int i = 0;

  // dump path 
  char *p = get_exe_path(current->active_mm);
  
  // initialize write data structure
  struct file_write_data* fw_data = NULL;
  if (p)
  {
    char *dump_path = kzalloc(PATH_MAX, GFP_ATOMIC);
    if (dump_path)
    {
      strcpy(dump_path, p);
      // make exe_dump as result file 
      strcat(dump_path, ".dump");
      printk(KERN_ALERT "%s open \n", dump_path);
      fw_data = kzalloc(sizeof(struct file_write_data), GFP_ATOMIC);
      if (fw_data)
      {
        fw_data->file = file_open(dump_path, O_WRONLY | O_CREAT | O_TRUNC , 0644);
        if (fw_data->file == NULL)
          printk(KERN_ALERT "%s open failed\n",dump_path);
      }
    }
  }

  do {
		int j = 0;
    print_log(fw_data, "thread: %d\n", i);
    
    for (j = 0; j < num_online_cpus(); j++)
    {
      int base_number = MAX_TIME_COUNT * (task->profile_data.cpu_data[j].list_counts-1);
      print_log(fw_data, ">> cpu: %d initial_state: %d list_counts: %d\n", 
          j, 
          task->profile_data.cpu_data[j].initial_state,
          task->profile_data.cpu_data[j].list_counts);
      print_log(fw_data, ">>>> resume_cnt: %d suspend_cnt: %d\n",
            base_number + task->profile_data.cpu_data[j].head->resume_counts, 
            base_number + task->profile_data.cpu_data[j].head->suspend_counts); 
      print_taskprofile_list(fw_data, task->profile_data.cpu_data[j].head);
    }

		task = next_thread(task);
		i++;

	} while (task != master_thread);

  if (fw_data)
  {
    if (fw_data->file)
    {
      printk(KERN_ALERT "dump file closing..\n");
      file_sync(fw_data->file);
      file_close(fw_data->file);
    }
    kfree(fw_data);
  }

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

