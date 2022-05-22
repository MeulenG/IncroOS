BUILD_DIR	=			../build

#NASM - Assembly Compiler
ASMC 	  	= 			nasm
ASMC_ARGS 	= 			-f


all			: 			$(BUILD_DIR)/keyboard.o keyboard.o

$(BUILD_DIR)/keyboard.o: Keyboard.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c Keyboard.c

keyboard.o: Keyboard.c
	gcc -std=c99 -mcmodel=large -ffreestanding -fno-stack-protector -mno-red-zone -c Keyboard.c

clean:
	rm -rf Keyboard.o
	rm -rf $(BUILD_DIR)/Keyboard.o