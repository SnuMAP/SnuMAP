//------------------------------------------------------------------------------
/// @brief Application trace profiler header
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/04 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription sc_profiler.h
///
//------------------------------------------------------------------------------

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

// for checking linux version (ioctl or unlocked_ioctl)
#include <linux/version.h>

// kernel version dependency
#if LINUX_VERSION_CODE > KERNEL_VERSION(3,0,0)
#define UNLOCKED
#define AMD64
#elif LINUX_VERSION_CODE > KERNEL_VERSION(2,6,36)
#define UNLOCKED
#define TILEGX
#endif

struct taskprofile_user_data {
	int resume_cnt[64];
	int suspend_cnt[64];
	unsigned long resume_time[64][10000];
	unsigned long suspend_time[64][10000];
};

struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns);
struct task_struct *find_task_by_vpid(pid_t vnr);
struct task_struct *find_process_by_pid(pid_t pid);

#define DEBUG_MODE 0
#define ACCESS_ONLY_ONE 0

#define IOCTL_START_PROFILING      _IOR(MAJOR_NUM, 1, NULL)
#define IOCTL_STOP_PROFILING       _IOR(MAJOR_NUM, 2, NULL)
#define IOCTL_DUMP_PROFILED_RESULT _IOR(MAJOR_NUM, 3, NULL)

#define IOCTL_COMMAND_1 IOCTL_START_PROFILING
#define IOCTL_COMMAND_2 IOCTL_STOP_PROFILING
#define IOCTL_COMMAND_3 IOCTL_DUMP_PROFILED_RESULT

#define DEVICE_FILE_NAME "profiler_mailbox"
#define MAJOR_NUM 101

#define SUCCESS 0

#if ACCESS_ONLY_ONE
static int profiler_opened = 0;
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
void start_profiling(void);
void stop_profiling(void);
void dump_profile_result(void);

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
