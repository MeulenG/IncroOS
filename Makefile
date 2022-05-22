BUILD_DIR	=			build/OS

# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb

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


all			:				STAGE1 STAGE2 CORE CPU LIBRARIES DRIVERS USERSPACE OSBUILD

.PHONY		: 				STAGE1 STAGE2 CORE CPU LIBRARIES DRIVERS USERSPACE USERSPACE2 USERSPACE3 OSBUILD

STAGE1:
	make -C Fat-Stage1

STAGE2:
	make -C Fat-Stage2

CORE:
	make -C Core

CPU:
	make -C cpu

DRIVERS:
	make -C drivers

LIBRARIES:
	make -C Libraries

USERSPACE:
	make -C Userspace

USERSPACE2:
	make -C Userspace2

USERSPACE3:
	make -C Userspace3

OSBUILD:
	ld -nostdlib -T link.lds -o kernel Core/kernel.o Core/main.o cpu/cputrapasm.o cpu/cputrap.o Libraries/libasm.o Libraries/printLib.o Core/kerneldebugger.o Libraries/memLib.o Core/process.o Core/syscall.o Libraries/lib.o drivers/Keyboard.o
	objcopy -O binary kernel kernel.bin 
	dd if=Fat-Stage1/FAT-STAGE1.bin of=boot.img bs=512 count=1 conv=notrunc
	dd if=Fat-Stage2/FAT-STAGE2.bin of=boot.img bs=512 count=5 seek=1 conv=notrunc
	dd if=kernel.bin of=boot.img bs=512 count=100 seek=6 conv=notrunc
	dd if=Userspace/user1.bin of=boot.img bs=512 count=10 seek=106 conv=notrunc
#	dd if=Userspace2/user2.bin of=boot.img bs=512 count=10 seek=116 conv=notrunc
#	dd if=Userspace3/user3.bin of=boot.img bs=512 count=10 seek=126 conv=notrunc

OSRUN:
	bochs -q -f bochsrc

clean:
	make -C Fat-Stage1 clean
	make -C Fat-Stage2 clean
	make -C Core clean
	make -C cpu clean
	make -C drivers clean
	make -C Libraries clean
	make -C Userspace clean
	make -C Userspace2 clean
	make -C Userspace3 clean
	rm -rf boot.img.lock
	rm -rf kernel
	rm -rf kernel.bin