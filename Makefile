# Compiler settings
TOOLS_DIR = ~/opt/cross/bin
TARGET = i686-elf
GCC = ${TOOLS_DIR}/${TARGET}-gcc
LD = ${TOOLS_DIR}/${TARGET}-ld

LINKER = ./src/linker.ld

STAGE1_ASM = ./src/boot/boot.asm
STAGE1_BIN = ./bin/stage1.bin

KERNEL_BIN = ./bin/kernel.bin
KERNEL_LKD = ./bin/kernel_linked.o

DISK_BIN = ./bin/disk.bin

C_SOURCES = $(shell find . -name "*.c")
C_HEADERS = $(shell find . -name "*.h")
C_OBJECTS = ${C_SOURCES:.c=.o}


all: clean build
test: all run
build: ${DISK_BIN}

stage1: ${STAGE1_BIN}

clean:
	find . -name \*.o -type f -delete
	find . -name \*.bin -type f -delete

run: ./bin
	qemu-system-i386 -drive file=${DISK_BIN},format=raw,index=0,media=disk

debug: all
	gdb -q -x gdb_cmd

${DISK_BIN}: ${KERNEL_BIN} stage1
	rm -rf ${DISK_BIN}
	dd if=${STAGE1_BIN} >> ${DISK_BIN}
	dd if=${KERNEL_BIN} >> ${DISK_BIN}

	dd if=/dev/zero count=8 bs=512 >> ${DISK_BIN}


${KERNEL_BIN}: ${C_OBJECTS}
	${LD} -g $(C_OBJECTS) -o ${KERNEL_LKD}
	${GCC} -g -T ${LINKER} -o ${KERNEL_BIN} -ffreestanding -O0 -nostdlib ${KERNEL_LKD}

${STAGE1_BIN}: 
	nasm -f bin ${STAGE1_ASM} -o ${STAGE1_BIN}

%.o: %.c
	${GCC} -ffreestanding -O0 -Wall -Wextra -c $< -o $@