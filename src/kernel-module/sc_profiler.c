//------------------------------------------------------------------------------
/// SnuMAP Application trace profiler implementation
///

/*
 * sc_profiler.c
 */
#include "sc_profiler.h"

// Helper Functions

// print_taskprofile_list function
// @param
// fw_data: file write data structure (target file informations(name, offset, etc.))
// initial_state: from saved profile data. it depends on task state when start profile. (suspend or resume.)
// thread_number:
// cpu_number:
// tp_current: current profile data list. (for DFS)
// @return
// list entry count
// @summary
// print list data using DFS
// @FIXME
// you have to be carefule to write recursive function since stack size is limited in kernel.
// iteration is more efficient.
int print_taskprofile_list(struct file_write_data* fw_data, 
			int initial_state, 
			int thread_number, 
			int cpu_number, 
			struct taskprofile_list *tp_current, 
			struct task_struct *task)
{
	int i = 0;
	int list_number = 0;
	if (tp_current == NULL) return 0;
	else list_number = print_taskprofile_list(fw_data, initial_state, thread_number, cpu_number, tp_current->next, task);

#if COALESCE
	for (i = ((list_number == 0 && initial_state <0 ) ? 1 : 0); i < tp_current->suspend_counts; i++) {
		if (tp_current->resume_time[i] == tp_current->suspend_time[i]) continue;
		if (first_log_flag == 1) {
			last_cpu_number = cpu_number;
			last_resume_time = tp_current->resume_time[i];
			last_suspend_time = tp_current->suspend_time[i];
			last_pid = task->pid;
			first_log_flag = 0;
		}

		else if ((last_cpu_number == cpu_number) && (last_pid == task->pid)) {
			if (last_suspend_time == tp_current->resume_time[i]) {
				last_suspend_time = tp_current->suspend_time[i];
			}

			else if (last_resume_time == tp_current->suspend_time[i]) {
				last_resume_time = tp_current->resume_time[i];
			}

			else {
				print_log(fw_data, "%d\t%llu\t%llu\t%s\t%ld\n",
						last_cpu_number,
						last_resume_time,
						last_suspend_time,
						fw_data->file_name,
						last_pid
					);
				last_cpu_number = cpu_number;
				last_resume_time = tp_current->resume_time[i];
				last_suspend_time = tp_current->suspend_time[i];
				last_pid = task->pid;
			}
		}

		else {
			print_log(fw_data, "%d\t%llu\t%llu\t%s\t%ld\n",
					last_cpu_number,
					last_resume_time,
					last_suspend_time,
					fw_data->file_name,
					last_pid
				);
			last_cpu_number = cpu_number;
			last_resume_time = tp_current->resume_time[i];
			last_suspend_time = tp_current->suspend_time[i];
			last_pid = task->pid;
		}
	}

	if (first_log_flag == 0) {
		print_log(fw_data, "%d\t%llu\t%llu\t%s\t%ld\n",
				last_cpu_number,
				last_resume_time,
				last_suspend_time,
				fw_data->file_name,
				last_pid
			);
		first_log_flag = 1;
	}

	return ++list_number;

#else
	for (i = ((list_number == 0 && initial_state <0 ) ? 1 : 0); i < tp_current->suspend_counts; i++) {
		if (tp_current->resume_time[i] == tp_current->suspend_time[i]) continue;
		print_log(fw_data, "%d\t%llu\t%llu\t%s\t%ld\n",
				cpu_number,
				tp_current->resume_time[i], 
				tp_current->suspend_time[i],
				fw_data->file_name,
				task->pid
			 );
	}
	return ++list_number;
#endif
}

/*
 char string helper functions
*/
char * _strcat(char *dest, const char *src)
{
	int i,j;
	for (i = 0; dest[i] != '\0'; i++)
		;
	for (j = 0; src[j] != '\0'; j++)
		dest[i+j] = src[j];
	dest[i+j] = '\0';
	return dest;
}

char * _strcpy(char *dest, const char *src)
{
	int i;
	for (i=0; src[i] != '\0'; i++)
		dest[i] = src[i];
	dest[i] = '\0';
	return dest;
}

char * _strrchr(char *dest, const char *src, const char delim)
{
	int i,j;
	j=0;
	for (i=0; src[i] != '\0'; i++)
	{
		if (src[i] == delim)
		{
			j = 0;
		}
		else
			dest[j++] = src[i]; 
	}
	dest[j] = '\0';
	return dest;
}

// _parse_path
// @param
// fw_data: target file_write_data structure to write path information.
// binary_path: current task binary path ( you can retrieve path using _get_binary_path function.
void _parse_path(struct file_write_data* fw_data, char* binary_path)
{
	_strcpy(fw_data->dump_path, binary_path);
	_strrchr(fw_data->file_name, binary_path, '/');
	// make exe_dump as result file 
	_strcat(fw_data->dump_path, ".csv"); //support csv file format
	//printk(KERN_ALERT "%s open \n", fw_data->dump_path);
}

// _get_binary_path
// @param
// mm: mm_struct structure in task structure
// @return
// binary full path
// @FIXME
// kfree?? pathname (allocated) will be freed?
// @reference
// d_path: https://www.kernel.org/doc/htmldocs/filesystems/API-d-path.html
char* _get_binary_path(struct mm_struct *mm)
{
	char *p = NULL;
	if (mm) {
		down_read(&mm->mmap_sem);
		if (mm->exe_file) {
			char *pathname = kzalloc(PATH_MAX, GFP_ATOMIC);
			if (pathname) {
				p = d_path(&mm->exe_file->f_path, pathname, PATH_MAX);
				//kfree(pathname); 
				// it raise kernel panic. 
				//i think d_path return value uses pathname buffer. (FIXME: when will you free this memory?)
			}
		}
		up_read(&mm->mmap_sem);
	}
	return p;
}


static int profiler_open(struct inode *inode, struct file *file)
{
#if ACCESS_ONLY_ONE
	if (profiler_opened)
		return -EBUSY;
	profiler_opened++;
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

/*
 file operations
 in this module, file_open, file_close, file_sync, file_write are only used.
*/
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

// print_log
// @summary
// genrally write log data to target file.
// it supports formatted string.
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
			printk(KERN_ALERT "dump file open failed. %s", buf);
		}
		else
		{
			// If profile_buffered)data is full, write it into file and empty it.
			if (profile_buffer_cnt + i > PROFILE_BUF_SIZE) 
			{
				// handle over uulong size. 
				// add file name postfix number.
				if (fw_data->offset + profile_buffer_cnt >= ULLONG_MAX)
				{
					const char c = '0' + fw_data->file_number++;
					file_sync(fw_data->file);
					file_close(fw_data->file);
					fw_data->file = NULL;
					fw_data->offset = 0;
					_strcat(fw_data->dump_path, &c);
					fw_data->file = file_open(fw_data->dump_path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);

					if (fw_data->file == NULL)
						printk(KERN_ALERT "%s open failed\n", fw_data->dump_path);
				}

				fw_data->offset += file_write(fw_data->file, fw_data->offset, profile_buffered_data, profile_buffer_cnt);
				profile_buffer_cnt = 0;
			}

			// Write contents of buf into local buffer profile_buffered_data.
			memcpy (profile_buffered_data + profile_buffer_cnt, buf, i);
			profile_buffer_cnt += i;
		}
		va_end(args);
		kfree(buf);
	}
}

static int start_profiling(void)
{
#if START_PROFILING_ONE
	if (profiler_started)
		return -EBUSY;
	profiler_started++;
#endif
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

#if START_PROFILING_ONE
	profiler_started--;
#endif

	return 0;
}

static int stop_profiling(void)
{
#if STOP_PROFILING_ONE
	if (profiler_stopped)
		return -EBUSY;
	profiler_stopped++;
#endif
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;

	do {
		task->profile_data.starting_flag = 0;
		task = next_thread(task);
	} while (task != master_thread);

#if STOP_PROFILING_ONE
	profiler_stopped--;
#endif

	return 0;
}

static int dump_profile_result(void)
{
#if DUMP_RESULT
	if (profiler_dumped)
		return -EBUSY;
	profiler_dumped++;
#endif
	struct task_struct* master_thread = current;
	struct task_struct* task = master_thread;
	int i = 0;
	int cpu_counts = num_online_cpus();

	// dump path 
	char *p = _get_binary_path(current->active_mm);

	// initialize write data structure
	struct file_write_data* fw_data = NULL;
    
	profile_buffered_data = kzalloc(PROFILE_BUF_SIZE, GFP_ATOMIC);
    profile_buffer_cnt = 0;

	if (p)
	{
		fw_data = kzalloc(sizeof(struct file_write_data), GFP_ATOMIC);
		if (fw_data)
		{
			_parse_path(fw_data, p);
			fw_data->file = file_open(fw_data->dump_path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC , 0644);
			if (fw_data->file == NULL)
				printk(KERN_ALERT "%s open failed\n",fw_data->dump_path);
		}
	}

	do {
		int j = 0;

		for (j = 0; j < cpu_counts; j++)
		{
			if (task->profile_data.cpu_data == NULL)
			{
				//printk(KERN_ALERT "[WARN] cpu_data is NULL when dump_profile_result - cpu counts %d %p\n", j, task->profile_data.cpu_data);
				continue;
			}
			print_taskprofile_list(fw_data, task->profile_data.cpu_data[j].initial_state, i, j, task->profile_data.cpu_data[j].head, task);
		}

		task = next_thread(task);
		i++;

	} while (task != master_thread);
			
	// handle over uulong size. 
	// add file name postfix number.
	if (fw_data->offset + profile_buffer_cnt >= ULLONG_MAX)
	{
		const char c = '0' + fw_data->file_number++;
		file_sync(fw_data->file);
		file_close(fw_data->file);
		fw_data->file = NULL;
		fw_data->offset = 0;
		_strcat(fw_data->dump_path, &c);
		fw_data->file = file_open(fw_data->dump_path, O_WRONLY | O_CREAT | O_TRUNC | O_SYNC, 0644);

		if (fw_data->file == NULL)
			printk(KERN_ALERT "%s open failed\n", fw_data->dump_path);
	}

	fw_data->offset += file_write(fw_data->file, fw_data->offset, profile_buffered_data, profile_buffer_cnt);

	kfree(profile_buffered_data);

	if (fw_data)
	{
		if (fw_data->file)
		{
			//      printk(KERN_ALERT "dump file closing..\n");
			file_sync(fw_data->file);
			file_close(fw_data->file);
		}
		kfree(fw_data);
	}

#if DUMP_RESULT
	profiler_dumped--;
#endif

	return 0;
}

unsigned long get_jiffies(void)
{
    unsigned long jiffies = get_jiffies_64();
    return jiffies_to_usecs(jiffies);
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
				ret = start_profiling();

				break;
			}
		case _IOC_NR(IOCTL_STOP_PROFILING):
			{
				printk(KERN_ALERT "stop_profiling called\n");
				ret = stop_profiling();

				break;
			}
		case _IOC_NR(IOCTL_DUMP_PROFILED_RESULT):
			{
				printk(KERN_ALERT "dump_profiled_result called\n");
				ret = dump_profile_result();
				printk(KERN_ALERT "dump_profiled_result finished\n");

				break;
			}
    case _IOC_NR(IOCTL_GET_JIFFIES):
        {
            //printk(KERN_ALERT "get_jiffies called\n");
            unsigned long jiffies = get_jiffies();
            ret = copy_to_user((void*)arg, &jiffies, sizeof(unsigned long));

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

