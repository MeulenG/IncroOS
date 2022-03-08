# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb
BUILD_DIR	=			../build


# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-std=c99
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-fno-stack-protector
GCC_ARGS 	+= 			-ffreestanding 
GCC_ARGS 	+= 			-m64
GCC_ARGS 	+= 			-mno-red-zone


all			: 			$(BUILD_DIR)/main_kernel.o


$(BUILD_DIR)/main_kernel.o: main_kernel.c
	gcc ${CFLAGS} ${GCC_ARGS} -c main_kernel.c -o $@

clean:
	rm $(BUILD_DIR)/*.o