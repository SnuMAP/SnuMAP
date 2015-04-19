//------------------------------------------------------------------------------
/// @brief Dynamic RTE kernel module common header
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr> and
///         Suwon Oh <suwon@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/02 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2014,2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription header.h
/// include SC shepherd common header
///
//------------------------------------------------------------------------------

#ifndef __SHEPHERD_HEADER_H
#define __SHEPHERD_HEADER_H

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

struct task_struct *find_task_by_pid_ns(pid_t nr, struct pid_namespace *ns);
struct task_struct *find_task_by_vpid(pid_t vnr);
struct task_struct *find_process_by_pid(pid_t pid);

#define DEBUG_MODE 0
#define ACCESS_ONLY_ONE 0

#endif
