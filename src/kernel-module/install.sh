#!/bin/sh
sudo insmod profiler_module.ko
sudo mknod $SNUMAP_ROOT/profiler_mailbox c 101 0
