#!/usr/bin/env sh
qemu-system-i386 -drive file=/IdeaProjects/GitOS/cmake-build-debug/disk.bin,format=raw,index=0,media=disk \
                 -m 32M \
                 -chardev stdio,id=char0,logfile=serial.log,signal=off \
                 -serial chardev:char0 \
                 -gdb tcp::1234 &
