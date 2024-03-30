# Compiler settings
TOOLS_DIR = /opt/cross/bin
TARGET = i686-elf
GCC = ${TOOLS_DIR}/${TARGET}-gcc
LD = ${TOOLS_DIR}/${TARGET}-ld
OBJCOPY = ${TOOLS_DIR}/${TARGET}-objcopy
OBJDUMP = ${TOOLS_DIR}/${TARGET}-objdump

BUILD_FOLDER = ./build

all: 
	-mkdir ${BUILD_FOLDER}
	nasm -f elf blank/blank.asm -o ./build/blank.o
	${GCC} -g -T ../userland_linker.ld -o blank.bin -ffreestanding -O0 -nostdlib -fpic -g ./build/blank.o
clean:
	rm -rf ${BUILD_FOLDER}