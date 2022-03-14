LIBC 		=			../libc
BUILD_DIR	=			../build


#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f
ASMC_ARGS 	+= 			elf64


# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-m64
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-fno-stack-protector

all: 	$(BUILD_DIR)/print.o $(BUILD_DIR)/lib.o	$(BUILD_DIR)/libassem.o	$(BUILD_DIR)/memory.o



$(BUILD_DIR)/print.o: print.c
	gcc ${CFLAGS} ${GCC_ARGS} -c print.c -o $@

$(BUILD_DIR)/lib.o: lib.h
	gcc ${CFLAGS} ${GCC_ARGS} -c lib.h -o $@

$(BUILD_DIR)/memory.o: memory.c
	gcc ${CFLAGS} ${GCC_ARGS} -c memory.c -o $@

$(BUILD_DIR)/libassem.o: lib.asm
	${ASMC} $< ${ASMC_ARGS} -o $@