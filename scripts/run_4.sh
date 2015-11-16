/opt/tilera/TileraMDE-4.1.5.154622/tilegx/bin/tile-monitor \
--rootfs /dev/sda1 \
--hvd MAX_CLIENTS=36 \
--hvd STRIPE_MEMORY=never \
--hvc /home/younghyun/tilera/hvc/vmlinux-4.hvc \
--hv-bin-dir /opt/tilera/TileraMDE-4.1.5.154622/tilegx/src/sys/hv \
--verbose
