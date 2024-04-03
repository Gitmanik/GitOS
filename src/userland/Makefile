BUILD_FOLDER = ./build

all: 
	-mkdir ${BUILD_FOLDER}
	nasm -f elf blank/blank.asm -o ./build/blank.o
	${GCC} -g -T ../userland_linker.ld -o build/blank.bin -ffreestanding -O0 -nostdlib -fpic ./build/blank.o
	
clean:
	rm -rf ${BUILD_FOLDER}

install:
	cp build/blank.bin ../../mnt
