# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb
include 				/home/puhaa/Desktop/OMOS/buildOS
include					/home/puhaa/Desktop/OMOS/buildpath
include					/home/puhaa/Desktop/OMOS/buildbootpath

# QEMU Emulator
EMU 	  	= 			qemu-system-i386
EMU_ARGS  	= 			-machine q35


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

all			:				Stage1 Stage2	cpu		drivers		kernel		kernel_entry	libc	$(BUILD_DIR_OS)/kernel.bin	$(BUILD_DIR_OS)/OS

.PHONY		: 				Stage1 Stage2	cpu		drivers		kernel		kernel_entry	libc	$(BUILD_DIR_OS)/kernel.bin	$(BUILD_DIR_OS)/OS

Stage1:
	make -C Stage1

Stage2:
	make -C Stage2

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

kernel.elf: build/kernel_entry.o
	i686-elf-ld -o $(BUILD_DIR_OS)/kernel.elf -Ttext 0x1000 build/kernel_entry.o build/kernel.o build/interrupt.o build/disk.o build/keyboard.o build/screen.o build/idt.o build/isr.o build/ports.o build/timer.o build/mem.o build/string.o


$(BUILD_DIR_OS)/OS:
	dd if=build/bootloader/fat-stage1.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=build/bootloader/fat-stage2.bin of=build/OS/OMOS-image.img bs=512 count=5 seek=1 conv=notrunc
	dd if=build/OS/kernel.bin of=build/OS/OMOS-image.img bs=512 count=100 seek=6 conv=notrunc


run: build/OS/OMOS-image.img
	qemu-system-i386 -drive if=virtio,file=build/OS/OMOS-image.img,format=raw -D ./log.txt -monitor stdio -smp 1 -m 4096


debug: OMOS-image.bin kernel.elf
	qemu-system-i386 -s -fda build/OS/OMOS-image.bin -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR_OS)/kernel.elf"

clean:
	make -C Stage1 clean 
	make -C cpu clean
	make -C drivers clean
	make -C kernel clean
	make -C kernel_entry clean
	make -C libc clean
	${RMVE} /home/puhaa/Desktop/OMOS/build/OS/*.bin