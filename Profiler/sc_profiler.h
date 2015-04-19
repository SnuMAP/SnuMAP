//------------------------------------------------------------------------------
/// @brief Many-SC profiler (kernel module) header
/// @author Younghyun Cho <younghyun@csap.snu.ac.kr>
/// Computer Systems and Platforms Laboratory
/// Department of Computer Science and Engineering
/// Seoul National University
///
/// @section changelog Change Log
/// 2015/01 Younghyun Cho created
///
/// @section license_section Licence
/// Copyright (c) 2014,2015 Computer Systems and Platforms Laboratory
/// All rights reserved.
/// 
/// @decription sc_profiler.h
/// communication between SC profiler and SC-RTE
///
//------------------------------------------------------------------------------

/*
 * sc_profiler.h
 */
#ifndef SC_profiler_H
#define SC_profiler_H

#include "header.h"
#include "profiler_interface.h"

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

/* kernel module */
int module_start(void);
void module_end(void);

#ifdef UNLOCKED
int profiler_ioctl(struct file *file,
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
