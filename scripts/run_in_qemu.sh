DISK_BIN=$1

qemu-system-i386 -S -gdb tcp::1234 -drive file=${DISK_BIN},format=raw,index=0,media=disk -m 32M -serial tcp:host.docker.internal:4555 -daemonize