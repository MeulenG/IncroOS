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


all			:				Fat-Stage1 Fat-Stage2 simulate

.PHONY		: 				Fat-Stage1 Fat-Stage2 simulate

Fat-Stage1:
	make -C Fat-Stage1

Fat-Stage2:
	make -C Fat-Stage2

simulate:
	$(osbuilder) buildos.yaml --target img

run:
	bochs -q -f setup.bochsrc

clean:
	make -C Fat-Stage1 clean
	rm -rf disk.img.lock
	rm -rf disk.img
	rm -rf bochsout.txt
