BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/Userstart3.o Userstart3.o Usermain3.o $(BUILD_DIR)/Usermain3.o Userlink

$(BUILD_DIR)/Userstart3.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

Userstart3.o: Userstart.asm
	${ASMC} $< ${ASMC_ARGS} elf64 -o $@

Usermain3.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

$(BUILD_DIR)/Usermain3.o: main.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c main.c

Userlink:
	ar -rcs lib.a ../Libraries/Userprint.o ../Libraries/syscall.o
	ar -rcs $(BUILD_DIR)/lib.a ../Libraries/Userprint.o ../Libraries/syscall.o
	ld -nostdlib -T linker.lds -o user Userstart3.o main.o lib.a
	ld -nostdlib -T linker.lds -o $(BUILD_DIR)/user Userstart3.o main.o lib.a
	objcopy -O binary user user3.bin
	objcopy -O binary user $(BUILD_DIR)/user3.bin

clean:
	rm -rf $(BUILD_DIR)/Userstart3.o
	rm -rf Userstart3.o
	rm -rf main.o
	rm -rf lib.a
	rm -rf $(BUILD_DIR)/lib.a
	rm -rf $(BUILD_DIR)/Usermain.o
	rm -rf user3.bin
	rm -rf $(BUILD_DIR)/user.bin
	rm -rf user
	rm -rf $(BUILD_DIR)/user
