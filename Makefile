# Good Idea for smart compilation later on when there is too many files to compile so it doesnt look like absolute garbage
C_SOURCES 	= $(wildcard SysCore/Core/*.c SysDrivers/*.c SysCore/cpu/*.c SysLib/libc/*.c)

HEADERS 	= $(wildcard SysCore/Core/*.h SysDrivers/keyboard/*.h SysCore/cpu/*.h SysLib/libc/*.h)

BUILD_DIR	=			/SysImage/boot.img

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


all			:			SysBoot SysCore build

.PHONY		: 			SysBoot SysCore build

SysBoot:
	make 	-C 			SysBoot

SysCore:
	make	-C			SysCore

build:
	dd 		if=build/bootloader/Stage1_X86.bin of=SysImage/boot.img bs=512 count=1 conv=notrunc
	
	dd 		if=build/bootloader/Stage2_X86.bin of=SysImage/boot.img bs=512 count=5 seek=1 conv=notrunc
	
	dd 		if=build/bootloader/Kernel_X86.bin of=SysImage/boot.img bs=512 count=100 seek=6 conv=notrunc

run:
	bochs 	-q -f SysEmulation/bochsrc

clean:
	make 	-C SysBoot clean

	make 	-C SysCore clean
	
	rm 		-rf SysImage/boot.img.lock
	
	rm 		-rf bochsout.txt
	
	rm 		-rf .vscode/configurationCache.log
	
	rm 		-rf .vscode/dryrun.log
	
	rm		-rf .vscode/targets.log
# Gotta Figure out how tf this line works when i start working with C files
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} -c $< -o $@