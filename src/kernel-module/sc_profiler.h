//------------------------------------------------------------------------------
/// SnuMAP Application trace profiler header
///

/*
 * sc_profiler.h
 */
#ifndef SC_profiler_H
#define SC_profiler_H

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/init.h>
#include <linux/fs.h>

#include <linux/sched.h>
#include <linux/cpumask.h>
#include <linux/bitmap.h>
#include <linux/migrate.h>

#include <linux/smp.h>
#include <linux/threads.h>
#include <linux/rcupdate.h>
#include <linux/cpu.h>
#include <linux/cpuset.h>
#include <linux/percpu.h>
#include <linux/errno.h>
#include <linux/migrate.h>
#include <linux/pid_namespace.h>

#include <linux/ioctl.h>

#include <asm/uaccess.h>

#include <linux/mempolicy.h>
#include <linux/slab.h>
#include <linux/nodemask.h>

#include <linux/jiffies.h>

// for checking linux version (ioctl or unlocked_ioctl)
#include <linux/version.h>

// kernel version dependency
// currently supported versions are:
//  AMD32/64: 3.19
//  TILEGX  : 3.14
#if defined(__x86_64__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,19,0))
#define UNLOCKED
#define AMD
#endif

#if defined(__tilegx__) && (LINUX_VERSION_CODE >= KERNEL_VERSION(3,14,0))
#define UNLOCKED
#define TILEGX
#endif

// file operation
#ifdef AMD
#include <asm/segment.h>
#else // TILEGX
#include <asm-generic/segment.h>
#endif

#include <linux/buffer_head.h>
struct file* file_open(const char* path, int flags, int rights);
void file_close(struct file* file);
int file_read(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);
int file_write(struct file* file, unsigned long long offset, unsigned char* data, unsigned int size);
int file_sync(struct file* file);

struct file_write_data
{
	struct file* file;
	unsigned long long offset;
	unsigned long long file_number;
	char dump_path[PATH_MAX];
	char file_name[PATH_MAX];
};
void print_log(struct file_write_data* fw_data, const char *fmt, ...);

#define COALESCE 1

int first_log_flag = 1;
int last_cpu_number = -1;
unsigned long last_resume_time = 0;
unsigned long last_suspend_time = 0;
pid_t last_pid = -1;

// ioctl definition
#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)
#define IOCTL_GET_JIFFIES          _IOR(MAJOR_NUM, 4, NULL)

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns);
struct task_struct *find_task_by_vpid(pid_t vnr);
struct task_struct *find_process_by_pid(pid_t pid);

#define DEBUG_MODE 0
#define ACCESS_ONLY_ONE 0
#define START_PROFILING_ONE 1
#define STOP_PROFILING_ONE 1
#define DUMP_RESULT 1

#define SUCCESS 0

#if ACCESS_ONLY_ONE
static int profiler_opened = 0;
#endif

#if START_PROFILING_ONE
static int profiler_started = 0;
#endif

#if STOP_PROFILING_ONE
static int profiler_stopped = 0;
#endif

#if DUMP_RESULT
static int profiler_dumped = 0;
#endif

/* basic kernel module implementation */
static int     profiler_open(struct inode *inode, struct file *file);
static int     profiler_release(struct inode *inode, struct file *file);
static ssize_t profiler_read(struct file *file,
		char __user *buffer,
		size_t length,
		loff_t *offset);
static ssize_t profiler_write(struct file *file,
		const char __user *buffer,
		size_t length,
		loff_t *offset);

/* basic functionalities implementation */
static int start_profiling(void);
static int stop_profiling(void);
static int dump_profile_result(void);

/* kernel module */
int module_start(void);
void module_end(void);

#ifdef UNLOCKED
long profiler_ioctl(struct file *file,
		unsigned int ioctl_num,
		unsigned long ioctl_param);

/* Module Declarations */
/*
 * This structure will hold the functions to be called
 * when a process does something to the device we created.
 * ...
 */
struct file_operations Fops = {
	.read  = profiler_read,
	.write = profiler_write,
	.unlocked_ioctl = profiler_ioctl,
	.open  = profiler_open,
	.release = profiler_release, /* a.k.a. close */
};

#else
int profiler_ioctl(struct inode *inode, /* see include/linux/fs.h */
		struct file *file,
		unsigned int ioctl_num,
		unsigned long ioctl_param);

/* Module Declarations */
/*
 * This structure will hold the functions to be called
 * when a process does something to the device we created.
 * ...
 */
struct file_operations Fops = {
	.read  = profiler_read,
	.write = profiler_write,
	.ioctl = profiler_ioctl,
	.open  = profiler_open,
	.release = profiler_release, /* a.k.a. close */
};

#endif
#endif

#define PROFILE_BUF_SIZE 4096
char *profile_buffered_data;
int profile_buffer_cnt = 0;

