# Compiler settings
TOOLS_DIR = /opt/cross/bin
TARGET = i686-elf
GCC = ${TOOLS_DIR}/${TARGET}-gcc
LD = ${TOOLS_DIR}/${TARGET}-ld
OBJCOPY = ${TOOLS_DIR}/${TARGET}-objcopy
OBJDUMP = ${TOOLS_DIR}/${TARGET}-objdump

SOURCE_PREFIX = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
GCC_ARGUMENTS = -g -std=gnu99 -ffreestanding -nostdlib -O0 -Wall -Wextra
#  -D__FILENAME__=\"$(notdir $<)\"

QEMU_ARGUMENTS = -drive file=${DISK_BIN},format=raw,index=0,media=disk -m 32M -serial /dev/ttyS0

STAGE1_ASM = ./src/boot/boot.asm
STAGE1_BIN = ./bin/stage1.bin

KERNEL_ELF = ./bin/kernel.elf
KERNEL_BIN = ./bin/kernel.bin

DISK_BIN = ./bin/disk.bin

C_SOURCES = $(shell find . -name "*.c")
C_HEADERS = $(shell find . -name "*.h")
C_OBJECTS = ${C_SOURCES:.c=.o}

# pm_entry.asm needs to be first linked because of hard-copying bytes in the last step
ASM_SOURCES = ./src/kernel/pm_entry.asm $(shell find ./src/kernel -name "*.asm" ! -wholename "./src/kernel/pm_entry.asm") 
ASM_OBJECTS = ${ASM_SOURCES:.asm=.asm.o}


all: clean build disk
test: all run

stage1: ${STAGE1_BIN}
kernel: ${KERNEL_BIN}

clean:
	rm -rf docs
	-pkill -9 bochs
	-pkill -9 qemu
	find . -name \*.o -type f -delete
	find . -name \*.elf -type f -delete
	find . -name \*.bin -type f -delete
	rm -rf *.ans
	rm -rf *.out
	rm -rf *.lock
	rm -rf *.sym

	rm -rf mnt

debug_qemu:
	qemu-system-i386 ${QEMU_ARGUMENTS} -S -serial /dev/ttyS0 -gdb tcp::1234 

debug_bochs:
	bochs -q

run:
	qemu-system-i386 ${QEMU_ARGUMENTS}

build: kernel stage1

disk: build
	rm -rf ${DISK_BIN}.lock
	rm -rf ${DISK_BIN}
	dd if=${STAGE1_BIN} >> ${DISK_BIN}
	dd if=${KERNEL_BIN} >> ${DISK_BIN}

# 										16MB = 16777216 - 1
	 dd if=/dev/zero of=${DISK_BIN} seek=16777215 bs=1 count=1
	-mkdir mnt
	sudo /usr/bin/mount -t vfat -o fat=16 ./bin/disk.bin ./mnt
	sudo cp -r ./fs/. ./mnt/.
	sudo umount ./mnt
	rm -rf ./mnt

# ASM_OBJECTS need to be first because of pm_entry.asm
${KERNEL_ELF}: ${C_OBJECTS} ${ASM_OBJECTS}
	${GCC} -T ./src/linker.ld ${GCC_ARGUMENTS} -o ${KERNEL_ELF} ${ASM_OBJECTS} $(C_OBJECTS)

${KERNEL_BIN}: ${KERNEL_ELF}
	${OBJCOPY} -O binary ${KERNEL_ELF} ${KERNEL_BIN}
	nm ${KERNEL_ELF} | sort | sed 's/\ A\ /\ /g' > bochs_symbols.sym

${STAGE1_BIN}: 
	nasm -f bin ${STAGE1_ASM} -o ${STAGE1_BIN}

%.asm.o: %.asm
	nasm -g -f elf $< -o $@

%.o: %.c
	${GCC} -I "src/kernel" -c ${GCC_ARGUMENTS} $< -o $@

dump:
	rm -rf dump.ans
	${OBJDUMP} --demangle -w --visualize-jumps=extended-color -Mintel --prefix-addresses -d -f -t -s ${KERNEL_ELF} > dump.ans

docs: ./docs
	doxygen Doxyfile