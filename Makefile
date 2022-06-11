# Good Idea for smart compilation later on when there is too many files to compile so it doesnt look like absolute garbage
C_SOURCES = $(wildcard SysCore/Core/*.c SysDrivers/*.c SysCore/cpu/*.c SysLib/libc/*.c)
HEADERS = $(wildcard SysCore/Core/*.h SysDrivers/keyboard/*.h SysCore/cpu/*.h SysLib/libc/*.h)

BUILD_DIR	=			build/OS

# C Compiler
CC			=			gcc -std=c99

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

# NASM - Assembly Compiler
AS 	  		= 			nasm
AS_ARGS 	= 			-f


all			:				SysBoot SysCalls SysCore SysDrivers SysLib USERSPACE build

.PHONY		: 				SysBoot SysCalls SysCore SysDrivers SysLib USERSPACE build

SysBoot:
	make -C SysBoot

SysCalls:
	make -C SysCalls

SysCore:
	make -C SysCore

SysDrivers:
	make -C SysDrivers

SysLib:
	make -C SysLib

USERSPACE:
	make -C Userspace

USERSPACE2:
	make -C Userspace2

USERSPACE3:
	make -C Userspace3

build:
	ld -nostdlib -T linkers/link.lds -o $(BUILD_DIR)/kernel build/OS/kernel.o build/dependensies/main.o build/dependensies/cputrapasm.o build/dependensies/cputrap.o build/libasm.o build/dependensies/print.o build/dependensies/kerneldebugger.o build/dependensies/memory.o build/dependensies/process.o build/Syscall.o build/dependensies/lib.o build/dependensies/keyboard.o
	objcopy -O binary $(BUILD_DIR)/kernel $(BUILD_DIR)/kernel.bin 
	dd if=build/bootloader/FAT-STAGE1.bin of=$(BUILD_DIR)/boot.img bs=512 count=1 conv=notrunc
	dd if=build/bootloader/FAT-STAGE2.bin of=$(BUILD_DIR)/boot.img bs=512 count=5 seek=1 conv=notrunc
	dd if=build/OS/kernel.bin of=$(BUILD_DIR)/boot.img bs=512 count=100 seek=6 conv=notrunc
	dd if=Userspace/user1.bin of=$(BUILD_DIR)/boot.img bs=512 count=10 seek=106 conv=notrunc
#	dd if=Userspace2/user2.bin of=boot.img bs=512 count=10 seek=116 conv=notrunc
#	dd if=Userspace3/user3.bin of=boot.img bs=512 count=10 seek=126 conv=notrunc

run:
	bochs -q -f bochsrc

clean:
	make -C SysBoot clean
	make -C SysCore clean
	make -C SysDrivers clean
	make -C SysLib clean
	make -C Userspace clean
	rm -rf boot.img.lock
	rm -rf bochsout.txt
	rm -rf kernel
	rm -rf kernel.bin

%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@