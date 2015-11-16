/opt/tilera/TileraMDE-4.1.5.154622/tilegx/bin/tile-monitor \
--rootfs /dev/sda1 \
--vmlinux /home/younghyun/tilera/boot/vmlinux \
--hvc /home/younghyun/tilera/hvc/vmlinux.hvc \
--hvx dataplane=1-34 \
--hvd STRIPE_MEMORY=never \
--hvd MEMPROF_TILE=5,5 \
--verbose
