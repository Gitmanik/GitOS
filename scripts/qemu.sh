#!/usr/bin/env sh
qemu-system-i386 -drive file=/IdeaProjects/GitOS/cmake-build-debug/disk.bin,format=raw,index=0,media=disk -m 32M -serial stdio