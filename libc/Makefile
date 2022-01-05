C_SOURCES 	= 			$(wildcard libc/*.c)
HEADERS 	= 			$(wildcard libc/*.h)
# Nice syntax for file extension replacement
OBJ 		= 			${C_SOURCES:.c=.o cpu/interrupt.o} 

# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb

BUILD_DIR=../build

OS_SRCS := 				$(wildcard *.c)
OS_OBJS := 				$(patsubst %.c, $(BUILD_DIR)/%.o, $(OS_SRCS))

# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-fda
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-m32


all: 					$(OS_OBJS)


# Generic rules for wildcards
# To make an object, always compile from its .c
%.o: %.c ${HEADERS}
	${CC} ${CFLAGS} ${GCC_ARGS} -c $< -o $@



clean:
	rm $(BUILD_DIR)/*