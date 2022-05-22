BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			Userstart2.o Usermain2.o Userlink

$(BUILD_DIR)/Userstart2.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

Userstart2.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

Usermain2.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

$(BUILD_DIR)/Usermain2.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

Userlink:
	ar -rcs lib.a ../Libraries/libasm.o ../Libraries/Userprint.o ../Libraries/syscall.o
	ld -nostdlib -T linker.lds -o user Userstart2.o main.o lib.a
	objcopy -O binary user user2.bin

clean:
	rm -rf $(BUILD_DIR)/Userstart2.o
	rm -rf Userstart2.o
	rm -rf main.o
	rm -rf lib.a
	rm -rf $(BUILD_DIR)/lib.a
	rm -rf $(BUILD_DIR)/Usermain.o
	rm -rf user2.bin
	rm -rf $(BUILD_DIR)/user.bin
	rm -rf user
	rm -rf $(BUILD_DIR)/user
