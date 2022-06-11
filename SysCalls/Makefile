BUILD_DIR	=			../build

# C Compiler
CC  		= 			gcc -std=c99

# -g: Use debugging symbols in gcc
CFLAGS 	 	= 			-g
CC_ARGS 	= 			-ffreestanding
CC_ARGS 	+= 			-Wall
CC_ARGS 	+= 			-Wextra
CC_ARGS 	+= 			-mcmodel=large
CC_ARGS 	+= 			-mno-red-zone
CC_ARGS 	+= 			-Werror
CC_ARGS 	+= 			-fno-exceptions
CC_ARGS 	+= 			-m64
CC_ARGS 	+= 			-fno-builtin
CC_ARGS 	+= 			-fno-stack-protector

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f

all			: 			$(BUILD_DIR)/syscall.o $(BUILD_DIR)/Syscall.o

$(BUILD_DIR)/syscall.o: Syscall.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR)/Syscall.o: syscall.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR)/syscall.o
	rm -rf $(BUILD_DIR)/Syscall.o