# Compiler settings
TOOLS_DIR = /opt/cross/bin
TARGET = i686-elf
GCC = ${TOOLS_DIR}/${TARGET}-gcc
LD = ${TOOLS_DIR}/${TARGET}-ld
OBJCOPY = ${TOOLS_DIR}/${TARGET}-objcopy
OBJDUMP = ${TOOLS_DIR}/${TARGET}-objdump

SOURCE_PREFIX = $(dir $(abspath $(lastword $(MAKEFILE_LIST))))
GCC_ARGUMENTS = -g -std=gnu99 -ffreestanding -nostdlib -O0 -Wall -Wextra -Werror

QEMU_ARGUMENTS = -drive file=${DISK_BIN},format=raw,index=0,media=disk -m 32M

SOURCE_FOLDER = ./src
BUILD_FOLDER = ./build

STAGE1_ASM = ./src/boot/boot.asm
STAGE1_BIN = ./build/stage1.bin

KERNEL_ELF = ./build/kernel.elf
KERNEL_BIN = ./build/kernel.bin

DISK_BIN = ./build/disk.bin
LO_DEV = /dev/loop0

C_SOURCES = $(shell find . -name "*.c")
C_HEADERS = $(shell find . -name "*.h")
C_OBJECTS = ${C_SOURCES:.c=.o}

# kernel.asm needs to be linked first because of hard-copying bytes in the last step
ASM_SOURCES = ./src/kernel/kernel.asm $(shell find ./src/kernel -name "*.asm" ! -wholename "./src/kernel/kernel.asm") 
ASM_OBJECTS = ${ASM_SOURCES:.asm=.asmo}

USERMODE_MAKE = cd ./src/userland && ${MAKE}


default: all
all: clean build userland disk
test: all run

stage1: ${STAGE1_BIN}
kernel: ${KERNEL_BIN}

clean: userland_clean
	rm -rf ${BUILD_FOLDER}
	rm -rf docs
	-pkill -9 bochs
	-pkill -9 qemu
	find . -name \*.o -type f -delete
	find . -name \*.asmo -type f -delete
	find . -name \*.elf -type f -delete
	find . -name \*.bin -type f -delete
	rm -rf *.ans
	rm -rf *.out
	rm -rf *.lock
	rm -rf *.sym

	rm -rf mnt

debug_qemu:
	qemu-system-i386 ${QEMU_ARGUMENTS} -S -gdb tcp::1234 

debug:
	gdb -ex 'target remote | qemu-system-i386 ${QEMU_ARGUMENTS} -S -gdb stdio'

debug_bochs:
	bochs -q

run:
	qemu-system-i386 ${QEMU_ARGUMENTS} -serial stdio

build: kernel stage1

disk: build
	# -losetup -d ${LO_DEV}
	-rm ${DISK_BIN}
	dd if=${STAGE1_BIN} >> ${DISK_BIN}
	dd if=${KERNEL_BIN} >> ${DISK_BIN}

# 							     16MB = 16777216 - 1
	dd if=/dev/zero of=${DISK_BIN} seek=16777215 bs=1 count=1
	rm -rf mnt
	-mkdir mnt

	mount -t vfat -o fat=16 ./build/disk.bin ./mnt
	cp -r ./fs/. ./mnt/.

	${USERMODE_MAKE} install

	sudo umount ./mnt
	rm -rf ./mnt

# ASM_OBJECTS need to be first because of kernel.asm
${KERNEL_ELF}: ${C_OBJECTS} ${ASM_OBJECTS}
	-mkdir build
	${GCC} -T ./src/kernel_linker.ld ${GCC_ARGUMENTS} -o ${KERNEL_ELF} ${ASM_OBJECTS} $(C_OBJECTS)

${KERNEL_BIN}: ${KERNEL_ELF}
	${OBJCOPY} -O binary ${KERNEL_ELF} ${KERNEL_BIN}
	nm ${KERNEL_ELF} | sort | sed 's/\ A\ /\ /g' > bochs_symbols.sym

${STAGE1_BIN}: 
	nasm -I ./src/boot -f bin ${STAGE1_ASM} -o ${STAGE1_BIN}

%.asmo: %.asm
	nasm -g -f elf $< -o $@

%.o: %.c
	${GCC} -I "src/kernel" -c ${GCC_ARGUMENTS} $< -o $@

dump:
	rm -rf dump.ans
	${OBJDUMP} --demangle -w --visualize-jumps=extended-color -Mintel --prefix-addresses -d -f -t -s ${KERNEL_ELF} > dump.ans

docs: ./docs
	doxygen Doxyfile

userland:
	${USERMODE_MAKE} all

userland_clean:
	${USERMODE_MAKE} clean