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


all			:				SysBoot

.PHONY		: 				SysBoot

SysBoot:
	make -C SysBoot

build:
	dd if=build/bootloader/FAT-STAGE1.bin of=$(BUILD_DIR)/boot.img bs=512 count=1 conv=notrunc
	dd if=build/bootloader/FAT-STAGE2.bin of=$(BUILD_DIR)/boot.img bs=512 count=5 seek=1 conv=notrunc

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