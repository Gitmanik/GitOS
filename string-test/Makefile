bin_name := boot.bin
asm_name := string_print_test.asm

all: build dump

clean:
	rm ${bin_name}

build:
	nasm -f bin ${asm_name} -o ${bin_name}

dump:
	objdump -D -Mintel,i8086 -b binary -m i386 ${bin_name}

run: all
	qemu-system-i386 -drive file=${bin_name},format=raw,index=0,media=disk
