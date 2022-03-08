BUILD_DIR	=			../build

# Environmental Variables
CC  		= 			/home/puhaa/opt/cross/bin/i686-elf-gcc
GDB 		= 			/home/puhaa/opt/cross/bin/i686-elf-gdb


#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f
ASMC_ARGS 	+= 			elf64

# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
GCC_ARGS 	= 			-ffreestanding
GCC_ARGS 	+= 			-Wall
GCC_ARGS 	+= 			-Wextra
#GCC_ARGS 	+= 			-Werror
GCC_ARGS 	+= 			-fno-exceptions
GCC_ARGS 	+= 			-fno-builtin
GCC_ARGS 	+= 			-fno-stack-protector



all			: 			$(BUILD_DIR)/trapa.o $(BUILD_DIR)/trap.o



$(BUILD_DIR)/trapa.o: trap.asm
	${ASMC} $< ${ASMC_ARGS} -o $@

$(BUILD_DIR)/trap.o: trap.c trap.h
	gcc ${CFLAGS} ${GCC_ARGS} -c trap.c -o $@

clean:
	rm $(BUILD_DIR)/*.o