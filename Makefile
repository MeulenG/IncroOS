BUILD_DIR	=			../build/OS

# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb


osbuilder	=			/home/puhaa/Desktop/GithubOpenSource/ValiOS/diskbuilder/build/osbuilder

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
AS 	  	= 			nasm
AS_ARGS 	= 			-f


all			:				Fat-Stage1 Fat-Stage2	cpu	Core	kernel_entry	libc	$(BUILD_DIR)/kernel	$(BUILD_DIR)/kernel.bin	simulate

.PHONY		: 				Fat-Stage1 Fat-Stage2	cpu	Core	kernel_entry	libc	$(BUILD_DIR)/kernel	$(BUILD_DIR)/kernel.bin	simulate

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
	ld -s -T link.lds -o kernel build/kernel_entry.o build/main_kernel.o build/trapa.o build/trap.o build/libassem.o build/print.o build/debug.o

$(BUILD_DIR)/kernel.bin:
	objcopy -O binary kernel kernel.bin 
#tmp way to build the OS and run it
$(BUILD_DIR)/OMOS-image.img:
	dd if=/home/puhaa/Desktop/DevProjects/OMOS/Fat-Stage1/fat-stage1.bin of=OMOS-image.img bs=512 count=1 conv=notrunc
	dd if=/home/puhaa/Desktop/DevProjects/OMOS/Fat-Stage2/fat-stage2.bin of=OMOS-image.img bs=512 count=5 seek=1 conv=notrunc
	dd if=/home/puhaa/Desktop/DevProjects/OMOS/build/kernel_entry.o of=OMOS-image.img bs=512 count=100 seek=6 conv=notrunc

#gonna use this later
simulate:
	$(osbuilder) --project "/home/puhaa/Desktop/DevProjects/OMOS/buildos.yaml" --target img
	qemu-system-x86_64 -cpu qemu, pdpe1gb -drive if=virtio,file=disk.img,format=raw -D ./log.txt -monitor stdio -smp 1 -m 4096


debug: OMOS-image.bin kernel.elf
	qemu-system-i386 -s -drive if=virtio,file=build/OS/OMOS-image.img,format=raw -D ./log.txt -monitor stdio -smp 1 -m 4096 -d guest_errors,int &
	${GDB} -ex "target remote localhost:1234" -ex "symbol-file $(BUILD_DIR_OS)/kernel.elf"

clean:
	make -C Fat-Stage1 clean
	make -C cpu clean
	make -C Core clean
	make -C kernel_entry clean
	rm -rf disk.img
	rm -rf kernel.bin
	rm -rf kernel
