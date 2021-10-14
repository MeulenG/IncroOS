GFLAGS=-m32
CCFLAGS =-std=c11 -02 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CCFLAGS+=-Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter
ASFLAGS=

LDFLAGS=-nostdlib -ffreestanding

boot: Stage1.asm
	nasm -f bin -o boot.bin Stage1.asm

iso: 
	dd if=/dev/zero of=boot.iso bs=512 count=2880
	dd if=boot.bin of=boot.iso conv=notrunc bs=512 count=1

create:
	mkdir iso
	cp floppy.img iso/
	genisoimage -quiet -V 'MYOS' -input-charset iso8859-1 -o myos.iso -b floppy.img \
    	-hide floppy.img iso/

run:
	qemu-system-i386 -cdrom ./myos.iso

clean:
	rm ./**/*.o

