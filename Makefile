BUILD_DIR	=			../build/OS

# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb

# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-m64
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-fno-stack-protector


#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


RMVE 		= 			rm -rf

all			:				Fat-Stage1 Fat-Stage2	cpu	Core	kernel_entry	libc	$(BUILD_DIR_OS)/kernel	ObjCopy	$(BUILD_DIR_OS)/OS

.PHONY		: 				Fat-Stage1 Fat-Stage2	cpu	Core	kernel_entry	libc	$(BUILD_DIR_OS)/kernel	ObjCopy	$(BUILD_DIR_OS)/OS

Fat-Stage1:
	make -C Fat-Stage1

Fat-Stage2:
	make -C Fat-Stage2

cpu:
	make -C cpu

#drivers:
#make -C drivers

Core:
	make -C Core

kernel_entry:
	make -C kernel_entry

libc:
	make -C libc

$(BUILD_DIR)/kernel:
	ld -s -T link.lds -o kernel build/kernel_entry.o build/main_kernel.o build/trapa.o build/trap.o build/libassem.o build/print.o

ObjCopy:
	objcopy -O binary kernel kernel.bin 

$(BUILD_DIR)/OS:
	dd if=/home/puhaa/Desktop/DevProjects/OMOS/Fat-Stage1/fat-stage1.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=/home/puhaa/Desktop/DevProjects/OMOS/Fat-Stage2/fat-stage2.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
	dd if=/home/puhaa/Desktop/DevProjects/OMOS/build/kernel_entry.o of=boot.img bs=512 count=100 seek=6 conv=notrunc

simulate: build/OS/boot.img
	osbuilder --project "/home/puhaa/Desktop/DevProjects/OMOS/test.yaml" --target img
	qemu-system-i386 -drive if=virtio,file=build/OS/OMOS-image.img,format=raw -D ./log.txt -monitor stdio -smp 1 -m 4096


debug: OMOS-image.bin kernel.elf
	qemu-system-i386 -s -drive if=virtio,file=build/OS/OMOS-image.img,format=raw -D ./log.txt -monitor stdio -smp 1 -m 4096 -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR_OS)/kernel.elf"

clean:
	make -C Fat-Stage1 clean
	make -C cpu clean
	make -C Core clean
	make -C kernel_entry clean
	make -C libc clean
	${RMVE} /home/puhaa/Desktop/OMOS/build/OS/*.bin