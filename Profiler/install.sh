#!/bin/sh
sudo insmod profiler_module.ko
sudo mknod $PWD/profiler_mailbox c 101 0
