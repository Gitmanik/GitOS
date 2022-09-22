bin_name := boot.bin
asm_name := boot.asm

all: build dump

clean:
	rm ${bin_name}

build:
	nasm -f bin ${asm_name} -o ${bin_name}
	dd if=sector2.txt >> ${bin_name}
	dd if=/dev/zero bs=512 count=1 >> ${bin_name}

dump:
	objdump -D -Mintel,i8086 -b binary -m i386 ${bin_name}

run: all
	qemu-system-i386 -drive file=${bin_name},format=raw,index=0,media=disk
