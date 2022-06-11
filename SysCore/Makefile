BUILD_DIR_KRNLDR	=			../build/OS
BUILD_DIR_DE		=			../build/dependensies

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


all			: 			$(BUILD_DIR_KRNLDR)/kernel.o $(BUILD_DIR_DE)/main.o $(BUILD_DIR_DE)/kerneldebugger.o $(BUILD_DIR_DE)/process.o $(BUILD_DIR_DE)/cputrapasm.o $(BUILD_DIR_DE)/cputrap.o

$(BUILD_DIR_KRNLDR)/kernel.o: Core/kernel.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR_DE)/main.o: Core/main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/kerneldebugger.o: Core/kerneldebugger.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/process.o: PManagement/process.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/cputrap.o: cpu/cputrap.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/cputrapasm.o: cpu/cputrap.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

clean:
	rm -rf $(BUILD_DIR_KRNLDR)/kernel.o
	rm -rf $(BUILD_DIR_DE)/main.o
	rm -rf $(BUILD_DIR_DE)/debug.o
	rm -rf $(BUILD_DIR_DE)/cputrap.o
	rm -rf $(BUILD_DIR_DE)/cputrapasm.o