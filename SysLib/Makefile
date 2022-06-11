BUILD_DIR	=		../build
BUILD_DIR_DE=		../build/dependensies
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


all			: 			$(BUILD_DIR_DE)/libasm.o $(BUILD_DIR_DE)/print.o $(BUILD_DIR_DE)/memory.o $(BUILD_DIR_DE)/lib.o $(BUILD_DIR_DE)/Userprint.o

$(BUILD_DIR_DE)/libasm.o: libc/lib.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR_DE)/print.o: libc/Print/printLib.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/memory.o: libc/Memory/memLib.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/lib.o: libc/lib.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/Userprint.o: libc/Print/Userprint.c
	$(CC) $(CFLAGS) -c $< -o $@

clean:
	rm -rf $(BUILD_DIR_DE)/libasm.o
	rm -rf $(BUILD_DIR_DE)/printLib.o
	rm -rf $(BUILD_DIR_DE)/memLib.o
	rm -rf $(BUILD_DIR_DE)/syscall.o
	rm -rf $(BUILD_DIR_DE)/Userprint.o