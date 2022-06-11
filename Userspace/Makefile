BUILD_DIR	=			../build
BUILD_DIR_DE=			../build/dependensies

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


all			: 			 $(BUILD_DIR_DE)/Userstart.o $(BUILD_DIR_DE)/Usermain1.o $(BUILD_DIR_DE)/Userlink

$(BUILD_DIR_DE)/Userstart.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

$(BUILD_DIR_DE)/Usermain1.o: main.c
	$(CC) $(CFLAGS) -c $< -o $@

$(BUILD_DIR_DE)/Userlink:
	ar rcs ../build/dependensies/lib.a ../build/dependensies/print.o ../build/dependensies/Userprint.o ../build/syscall.o ../build/libasm.o
	ld -nostdlib -T ../linkers/linker.lds -o ../build/Userspace/user $(BUILD_DIR_DE)/Userstart.o $(BUILD_DIR_DE)/Usermain1.o ../build/dependensies/lib.a 
	objcopy -O binary ../build/Userspace/user ../build/Userspace/user1.bin

clean:
	rm -rf $(BUILD_DIR_DE)/Userstart.o
	rm -rf $(BUILD_DIR_DE)/lib.a
	rm -rf $(BUILD_DIR_DE)/Usermain.o
	rm -rf $(BUILD_DIR_DE)/user1.bin
	rm -rf $(BUILD_DIR_DE)/user