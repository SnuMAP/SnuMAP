/opt/tilera/TileraMDE-4.1.5.154622/tilegx/bin/tile-monitor \
--hvd MAX_CLIENTS=36 \
--rootfs /dev/sda1 \
--hvd STRIPE_MEMORY=never \
--hvc /home/younghyun/tilera/hvc/vmlinux-36-program.hvc \
--hv-bin-dir /opt/tilera/TileraMDE-4.1.5.154622/tilegx/src/sys/hv \
--greet-timeout 300 \
--verbose
