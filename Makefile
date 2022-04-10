BUILD_DIR	=			../build/OS

# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb


osbuilder	=			/home/puhaa/Desktop/GithubOpenSource/ValiOS/diskbuilder/build/osbuilder

# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
CC_ARGS 	= 			-ffreestanding
CC_ARGS 	+= 			-Wall
CC_ARGS 	+= 			-Wextra
CC_ARGS 	+= 			-Werror
CC_ARGS 	+= 			-fno-exceptions
CC_ARGS 	+= 			-m64
CC_ARGS 	+= 			-fno-builtin
CC_ARGS 	+= 			-fno-stack-protector


#NASM - Assembly Compiler
AS 	  		= 			nasm
AS_ARGS 	= 			-f


all			:				Fat-Stage1 Fat-Stage2	cpu	Core	kernel_entry	libc	disk.img

.PHONY		: 				Fat-Stage1 Fat-Stage2	cpu	Core	kernel_entry	libc	$(BUILD_DIR)/kernel	$(BUILD_DIR)/kernel.bin	disk.img

Fat-Stage1:
	make -C Fat-Stage1

Fat-Stage2:
	make -C Fat-Stage2

cpu:
	make -C cpu

drivers:
	make -C drivers

Core:
	make -C Core

kernel_entry:
	make -C kernel_entry

libc:
	make -C libc

$(BUILD_DIR)/kernel:
	ld -s -T link.lds -o kernel build/kernel_entry.o build/main_kernel.o build/trapa.o build/trap.o build/libassem.o build/print.o build/debug.o build/memory.o

$(BUILD_DIR)/kernel.bin:
	objcopy -O binary kernel kernel.bin 

#tmp way to build the OS and run it
disk.img:
	dd if=/dev/zero of=myfloppy.img bs=512 count=2880
	sudo losetup /dev/loop50 myfloppy.img 
	sudo mkdosfs -F 12 /dev/loop50
	sudo mount /dev/loop50 /media/myfloppy -t msdos -o "fat=12"
	sudo dd if=Fat-Stage1/STAGE1.SYS of=/dev/loop50
	sudo cp Fat-Stage2/KRNLDR.SYS myfloppy


simulate:
	$(osbuilder) buildos.yaml --target img

run:
	bochs -q -f bochsrc

clean:
	make -C Fat-Stage1 clean
	make -C Fat-Stage2 clean
	make -C cpu clean
	make -C Core clean
	make -C kernel_entry clean
	rm -rf disk.img
	rm -rf kernel.bin
	rm -rf kernel
	rm -rf disk.img.lock
	rm -rf myfloppy
	rm -rf myfloppy.img
	rm -rf bochsout.txt
	sudo umount /media/myfloppy
	sudo losetup -d /dev/loop50