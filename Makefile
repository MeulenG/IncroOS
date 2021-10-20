CC=gcc
NA=nasm
GFLAGS=-m32
CCFLAGS =-std=c11 -02 -g -Wall -Wextra -Wpedantic -Wstrict-aliasing
CCFLAGS+=-Wno-pointer-arith -Wno-newline-eof -Wno-unused-parameter
ASFLAGS=
PROGRAMS=boot.bin boot.iso
LDFLAGS=-nostdlib -ffreestanding

Bootloader: Bootloader/Stage1.asm
	$(NA) -f bin -o boot.bin Bootloader/Stage1.asm
	$(NA) -f bin Bootloader/Stage2.asm -o Stage2.sys 

iso: 
	dd if=/dev/zero of=boot.iso bs=512 count=2880
	dd if=boot.bin of=boot.iso conv=notrunc bs=512 count=1
	dd if=Stage2.sys of=boot.iso bs=512 seek=1 conv=notrunc skip=1


qemu:
	qemu-system-i386 -cdrom ./boot.iso

clean:
	rm -rf core *.o $(PROGRAMS)