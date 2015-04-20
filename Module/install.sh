#!/bin/sh
sudo insmod profiler_module.ko
sudo mknod $OMP_PROFILER_ROOT/profiler_mailbox c 101 0
