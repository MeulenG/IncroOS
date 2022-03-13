LIBC 		=			../libc
BUILD_DIR	=			../build


#NASM - Assembly Compiler
AS 	  	= 			nasm
AS_ARGS 	= 			-f
AS_ARGS 	+= 			elf64


# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
CC_ARGS 	= 			-ffreestanding
CC_ARGS 	+= 			-Wall
CC_ARGS 	+= 			-Wextra
CC_ARGS 	+= 			-Werror
CC_ARGS 	+= 			-fno-exceptions
CC_ARGS 	+= 			-fno-builtin
CC_ARGS 	+= 			-fno-stack-protector

all: 	$(BUILD_DIR)/print.o 	$(BUILD_DIR)/lib.o	$(BUILD_DIR)/libassem.o



$(BUILD_DIR)/print.o: print.c print.h
	gcc ${CFLAGS} ${CC_ARGS} -c print.c -o $@

$(BUILD_DIR)/lib.o: lib.h
	gcc ${CFLAGS} ${CC_ARGS} -c lib.h -o $@


$(BUILD_DIR)/libassem.o: lib.asm
	${AS} $< ${AS_ARGS} -o $@
