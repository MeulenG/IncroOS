# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb
include 				/home/puhaa/Desktop/PuhaaOS/buildOS
include					/home/puhaa/Desktop/PuhaaOS/buildpath
include					/home/puhaa/Desktop/PuhaaOS/buildbootpath

# Emulator
EMU 	  	= 			qemu-system-i386
EMU_ARGS  	= 			-machine q35
EMU_ARGS  	+= 			-fda


# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-m32
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-nostdlib
GCC_ARGS 	+= 			-nostdinc
GCC_ARGS 	+= 			-fno-stack-protector
GCC_ARGS 	+= 			-nostartfiles
GCC_ARGS 	+= 			-nodefaultlibs

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


RMVE 		= 			rm -rf

all			:			boot	cpu		drivers		kernel		kernel_entry	libc	$(BUILD_DIR_OS)/kernel.bin	$(BUILD_DIR_OS)/OS

.PHONY		: 			boot	cpu		drivers		kernel		kernel_entry	libc	$(BUILD_DIR_OS)/kernel.bin	$(BUILD_DIR_OS)/OS

boot:
	make -C boot

cpu:
	make -C cpu

drivers:
	make -C drivers

kernel:
	make -C kernel

kernel_entry:
	make -C kernel_entry

libc:
	make -C libc

$(BUILD_DIR_OS)/kernel.bin:
	i686-elf-ld -o $(BUILD_DIR_OS)/kernel.bin -Ttext 0x1000 build/kernel_entry.o build/kernel.o build/interrupt.o build/disk.o build/keyboard.o build/screen.o build/idt.o build/isr.o build/ports.o build/timer.o build/mem.o build/string.o --oformat binary

$(BUILD_DIR_OS)/OS:
	cat build/bootloader/Stage1.bin build/bootloader/Stage2.bin build/OS/kernel.bin > build/OS/PuhaaOS-image.bin

run: PuhaaOS-image.bin
	${EMU} ${EMU_ARGS} build/OS/PuhaaOS-image.bin


clean:
	make -C boot/Stage1 clean 
	make -C cpu clean
	make -C drivers clean
	make -C kernel clean
	make -C kernel_entry clean
	make -C libc clean