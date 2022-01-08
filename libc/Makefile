LIBC 		=			../libc
BUILD_DIR	=			../build

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
GCC_ARGS 	+= 			-m32
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-nostdlib
GCC_ARGS 	+= 			-nostdinc
GCC_ARGS 	+= 			-fno-stack-protector
GCC_ARGS 	+= 			-nostartfiles
GCC_ARGS 	+= 			-nodefaultlibs

all: $(BUILD_DIR)/string.o $(BUILD_DIR)/mem.o



$(BUILD_DIR)/string.o: string.c string.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c string.c -o $@

$(BUILD_DIR)/mem.o: mem.c mem.h
	${CC} ${CFLAGS} ${GCC_ARGS} -c mem.c -o $@