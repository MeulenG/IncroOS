BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			 Userstart.o main.o Userlink

$(BUILD_DIR)/Userstart.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

Userstart.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

main.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

$(BUILD_DIR)/Usermain.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

Userlink:
	ar rcs lib.a ../Libraries/printLib.o ../Libraries/Userprint.o ../Libraries/syscall.o ../Libraries/libasm.o
	ld -nostdlib -Tlinker.lds  -o user Userstart.o main.o lib.a 
	objcopy -O binary user user1.bin

clean:
	rm -rf $(BUILD_DIR)/Userstart.o
	rm -rf Userstart.o
	rm -rf main.o
	rm -rf lib.a
	rm -rf $(BUILD_DIR)/lib.a
	rm -rf $(BUILD_DIR)/Usermain.o
	rm -rf user1.bin
	rm -rf $(BUILD_DIR)/user1.bin
	rm -rf user
	rm -rf $(BUILD_DIR)/user
